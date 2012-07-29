/*
 * This file is part of XV-11 Parser
 *
 * XV-11 Parser is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include "parser.h"

using namespace std;

struct args_t {
    bool cli;           // true if -c is present
    char *filename;     // path to dump file (-f)
    char *serialport;   // path to serial port (-p)
    char *gifname;      // path to save gif (-g)
} args;

static const char *optstring = "cf:p:g:h?";

void displayUsage() {
    cout << "XV-11 Parser v0.1" << endl;
    cout << "Copyright (c) Robert Ying 2012" << endl;
    cout << "Released under the GPLv3" << endl;
    cout << endl;
    cout << "Usage:" << endl;
    cout << "\tparser [-c] -f dumpfile [-g gifname]" << endl;
    cout << "\tparser [-c] -p serialport [-g gifname]" << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << "\t-c\t\tCLI Mode; all output printed to stdout" << endl;
    cout << "\t-f\t\tPath to serial dump file" << endl;
    cout << "\t-p\t\tSerial device name" << endl;
    cout << "\t-g\t\tPath to save gif to" << endl;
    cout << "\t-h\t\tDisplay usage" << endl;
    cout << endl;
}

int main (int argc, char** argv) {
    args.cli = false;
    args.filename = NULL;
    args.serialport = NULL;
    args.gifname = NULL;

    char c;

    // process arguments
    while ((c = getopt(argc, argv, optstring)) != -1) {
        switch(c) {
            case 'c':
                args.cli = true;
                break;
            case 'f':
                args.filename = optarg;
                break;
            case 'p':
                args.serialport = optarg;
                break;
            case 'g':
                args.gifname = optarg;
                break;
            case 'h':
            case '?':
                displayUsage();
                return 1;
                break;
        }
    }

    if (args.filename && args.serialport) {
        cerr << "You can supply a path to a serial dump file OR a path to a serial port, not both" << endl;
        return 1;
    }

    if (!args.filename && !args.serialport) {
        displayUsage();
        return 1;
    }

    parser p("XV-11 Parser", !args.cli); 

    if (args.cli) {
        cout << "Running in command line mode" << endl;
    } else {
        cout << "Running in gui mode" << endl;
    }

    if (args.serialport) {
        cerr << "Serial ports not yet supported" << endl;
        return 1;
    } else if (args.filename) {
        cout << "Opening input file " << args.filename << endl;
        ifstream file(args.filename, ios::in | ios::binary | ios::ate);
        if (file.is_open()) {
            cout << "Parsing file..." << endl;
            ifstream::pos_type size = file.tellg();
            file.seekg(0, ios::beg);

            // simulating serial input here
            cout << "Iterating through chars" << endl;
            for (int index = 0; index < size; index++) {
                char c;
                file.read(&c, 1); // c now has new char
                p.update(c);
            }
            file.close();
        } else {
            cerr << "Could not open file " << args.filename << endl;
            return 1;
        }
    }

    if (args.gifname) {
        cout << "Writing gif to " << args.gifname << endl;
        p.writeMap(args.gifname);
    }
}
