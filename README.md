XV-11 Parser
============

*Author:* Robert Ying

History
-------
This program is designed to parse the output from SetStreamFormat packet.
Prompted by Chunk's tantalizing images of XV-11 maps, I decided it would be
interesting to be able to see them form... in an animation!

What's it do?
-------------
This program will interpret the binary-encoded packet format of the Neato XV-11
(started by SetStreamFormat packet), and output useful human-readable
information. It can currently parse three types of packets:

* packet 0x05: Laser data packet, prints distances of relevant sector to stdout
* packet 0x09: Map image packet, outputs to nice animated gif when complete
* packet 0x11: Text data packet, prints message to stdout

Usage
-----
Clone the repository, run make. When complete, there should be a program
"parser" in the bin directory, it has arguments as follows:

parser [path-to-serial-dump-file] [path-to-desired-gif-location]

Where the serial dump is a binary dump of the output over serial, and the
desired gif location is where you would like 

Note: You may need to create a bin/ and obj/ directory in the root of this
project in order for it to compile, as they are in the gitignore.

Dependencies
------------
Magick++

Install on Ubuntu with

`sudo apt-get install libmagick++-dev libmagick++4`

Special thanks
--------------
Hash79, Chunk, DaveC, Xevel, and all the other people on TrossenRobotics who've
spent time working on XV-11 hacks
