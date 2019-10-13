#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <algorithm>
#include <iterator>
#include <regex>
#include <fstream>
#include <cstdio>
#include <tlhelp32.h>

std::string getPlayCountFromStringInMemory(int* address, HANDLE process, MEMORY_BASIC_INFORMATION info, std::string spotifyTrackURI)
{
	// Scans memory to find the playcount given the track URI
	std::vector<char> buffer;
	DWORD bytesRead;

	buffer.resize(info.RegionSize);
	ReadProcessMemory(process, address, &buffer[0], info.RegionSize, &bytesRead);
	// 75 should be enough since the track ID is fixed size
	buffer.resize(75);

	std::string foundString = "";

	std::vector<char>::iterator pos;
	// Search for matching string in memory.
	for (std::vector<char>::const_iterator i = buffer.begin(); i != buffer.end(); ++i)
	{
		foundString = foundString + *i;
	}

	std::regex rgx("uri\": \"" + spotifyTrackURI + "\",\"playcount\": (.*),");
	std::smatch match;

	// Fetch playcount from the found string.
	if (std::regex_search(foundString, match, rgx))
	{
		return match.str(1);
	}

	return "NOT FOUND";
}

void writeToFile(std::string spotifyTrackID, std::string playCount)
{
	// Writes results to a CSV file.
	std::ofstream outputFile;
	outputFile.open("playcounts.csv", std::ios_base::app);
	outputFile << spotifyTrackID << ";" << playCount << std::endl;
	outputFile.close();
}

std::string findPlayCountStringInMemory(HANDLE process, std::string const& pattern, std::string spotifyTrackURI)
{

	unsigned char* p = NULL;
	MEMORY_BASIC_INFORMATION info;

	for (p = NULL;VirtualQueryEx(process, p, &info, sizeof(info)) == sizeof(info);p += info.RegionSize)
	{
		std::vector<char> buffer;
		std::vector<char>::iterator pos;

		if (info.State == MEM_COMMIT &&	(info.Type == MEM_MAPPED || info.Type == MEM_PRIVATE) && p != NULL)
		{
			DWORD bytes_read;
			buffer.resize(info.RegionSize);
			ReadProcessMemory(process, p, &buffer[0], info.RegionSize, &bytes_read);
			buffer.resize(bytes_read);
			for (pos = buffer.begin();buffer.end() != (pos = std::search(pos, buffer.end(), pattern.begin(), pattern.end()));++pos)
			{
				std::string foundPlayCount = getPlayCountFromStringInMemory((int*)(p + (pos - buffer.begin())),process,info, spotifyTrackURI);
				return foundPlayCount;
			}
		}
	}

	return "NOT FOUND";
}

DWORD FindProcessId(const std::wstring& processName)
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE) {
		return 0;
	}

	Process32First(processesSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile))
	{
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processesSnapshot, &processInfo))
	{
		if (!processName.compare(processInfo.szExeFile))
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	CloseHandle(processesSnapshot);
	return 0;
}



int main(int argc, char** argv) {
	
	std::string foundPlayCount;
	std::string spotifyTrackID = "";
	std::string spotifyTrackURI = "";

	// Searches for the PID of Spotify.exe

	DWORD processID = FindProcessId(L"Spotify.exe");

	if (processID == 0)
	{
		std::cout << "Could not find Spotify.exe, is Spotify open?" << std::endl;
		return 1;
	}

	std::cout << "Found Spotify.exe with PID: " << processID << std::endl;

	HANDLE process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, false, (DWORD) processID);

	std::ifstream trackList("tracks.csv");

	if (!trackList.is_open())
	{
		std::cout << "Can't find tracks.csv. The file is made by Spotify Track IDs separated by a newline" << std::endl;
		return 1;
	}

	// Construct the Spotify Track URI
	for (std::string line; getline(trackList, line); )
	{
		spotifyTrackID = line;
		spotifyTrackURI = "spotify:track:" + line;
		
		// Convert std::string to LPCWSTR.
		// Is that a better way? I don't know.
		std::wstring stemp = std::wstring(spotifyTrackURI.begin(), spotifyTrackURI.end());
		LPCWSTR sw = stemp.c_str();

		// Open the Spotify URI (Spotify should POP up)
		ShellExecute(NULL, L"open", sw, NULL, NULL, SW_SHOWNORMAL);
		// We just wait to be sure that the song is loaded on screen.
		Sleep(500);

		// Pattern to search for in memory
		// After visiting the URI Spotify loads in memory the result of the request made with the Hermes protocol.
		// We don't need to modify Spotify or figure out the protocol, we can just grab everything from memory.

		std::string pattern("uri\": \"" + spotifyTrackURI + "\",\"playcount\":");

		foundPlayCount = findPlayCountStringInMemory(process, pattern, spotifyTrackURI);
		std::cout << "Found " << foundPlayCount << " for " << spotifyTrackID << std::endl;
		writeToFile(spotifyTrackID, foundPlayCount);
	}

	std::cout << "All done!" << std::endl;


	return 0;
}