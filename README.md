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

    parser [-c] -f dumpfile [-g gifname]
    parser [-c] -p serialport [-g gifname]

Where the serial dump is a binary dump of the output over serial, and the
desired gif location is where you would like the gif of the map images to be
placed. 

Serial ports are not currently supported, but they're a goal of this project
nevertheless.

Dependencies
------------
Magick++

Install on Ubuntu with

`sudo apt-get install libmagick++-dev libmagick++4`

OpenCV

This code has been tested with the OpenCV libraries available in the ROS Fuerte
repository; however, any fairly recent version of OpenCV ought to work (ie
version 2.0+)

Special thanks
--------------
Hash79, Chunk, DaveC, Xevel, and all the other people on TrossenRobotics who've
spent time working on XV-11 hacks
