# UglySpotifyPlaycountFetcher
A simple program that scans memory to find a track playcount on Spotify.

**You will need Spotify installed and open while this is running. The Spotify window will pop on your screen for the first song and will blink in the taskbar for every other song**

## Why?

To build [IndieGuerraBot](https://github.com/PaaaulZ/IndieGuerraBot) I needed a way to find the playcount for a song but unfortunately Spotify doesn't provide one. At first I found a way but it involved editing Spotify files and I didn't like it. I wanted something to run and let go without having to think about it or touching Spotify's code/files.

## How does this work?

Honestly I'm not even sure it will work. It's working for me (tested on Windows 10 Pro 10.0.18362 compiling in x86 and x64) and that's good.

Spotify comunicates with the Hermes protocol and it's packets are encoded. At first I thought about computer vision but that sounded unreliable, so I thought about finding and detouring the function that encodes/decodes packet but:

1) That's too much work.
2) A software update could break my code.
3) Is against Spotify's ToS (?).

Since I was thinking about memory then I jumped to the conclusion that I didn't need to work that much. Seeing the playcount on my computer meant that I have it loaded in memory, having it loaded in memory meant that I could take it from there and do whatever I like with it.

## That's what it does?

Yes.
This program will search for a part (the start) of the response JSON that contains the playcount and when it finds the string will return the "numbers" and save them to a CSV file.

## Why is this named Ugly?

Seriously? 

1) I only tested this on Windows.
2) You will need Spotify installed and open just for this program. It's also a bit annoying because Spotify will pop open and gain focus on the first song the software asks for and then your taskbar will blink every time the software switches to another song.
3) The code is a mess and really simple, just like the concept.
4) It's a bit slow. On my computer it takes 2-3 seconds to get a song's playcount. Maybe in next versions I could start searching from a certain offset in memory to reduce the time it takes to scan.

But it works and it's ok for me, hope it will for you too.
