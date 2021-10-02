# Mus2Midi
A simple MUS to MIDI convertor

MUS files are found in games like DOOM.

This differs from other convertors in that it creates the MIDI data in memory and not on a HDD.

## Usage
The main.c file acts as a demonstration of how to use mus2midi, but no file I/O needs to take place depending on your needs.
The files mus2midi.c and mus2midi.h can be imported into your own project, thus keeping all data in memory.
Just remember to free the MIDI data using the returned pointer after you're finished with it.
