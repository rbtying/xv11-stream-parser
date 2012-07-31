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
#include <termios.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include "parser.h"

using namespace std;

struct args_t {
    bool cli;           // true if -c is present
    bool verbose;       // true if -v is present
    bool laser;         // true if -l is present
    bool text;          // true if -t is present
    bool map;           // true if -m is present
    bool odom;          // true if -o is present
    char *filename;     // path to dump file (-f)
    char *serialport;   // path to serial port (-p)
    char *gifname;      // path to save gif (-g)
} args;

static const char *optstring = "cvltmof:p:g:h?";

static const char *activation_cmd = "SetStreamFormat packet\r\n";
    
parser p("XV-11 Parser", false); 

bool done = false;

void displayUsage() {
    cout << "XV-11 Parser v0.1" << endl;
    cout << "Copyright (c) Robert Ying 2012" << endl;
    cout << "Released under the GPLv3" << endl;
    cout << endl;
    cout << "Usage:" << endl;
    cout << "\tparser [-cvltm] -f dumpfile [-g gifname]" << endl;
    cout << "\tparser [-cvltm] -p serialport [-g gifname]" << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << "\t-c\t\tCLI Mode; all output printed to stdout" << endl;
    cout << "\t-v\t\tVerbose; more data is printed to stdout" << endl;
    cout << "\t-l\t\tLaser messages printed to stdout" << endl;
    cout << "\t-t\t\tText messages printed to stdout" << endl;
    cout << "\t-m\t\tMap messages printed to stdout" << endl;
    cout << "\t-o\t\tOdometry messages printed to stdout" << endl;
    cout << "\t-f\t\tPath to serial dump file" << endl;
    cout << "\t-p\t\tSerial device name" << endl;
    cout << "\t-g\t\tPath to save gif to" << endl;
    cout << "\t-h\t\tDisplay usage" << endl;
    cout << endl;
}

void term(int signum) {
    done = true;
}

int main (int argc, char** argv) {
    args.cli = false;
    args.filename = NULL;
    args.serialport = NULL;
    args.gifname = NULL;
    args.laser = false;
    args.text = false;
    args.map = false;
    args.odom = false;
    args.verbose = false;

    char c;

    // process arguments
    while ((c = getopt(argc, argv, optstring)) != -1) {
        switch(c) {
            case 'c':
                args.cli = true;
                break;
            case 'v':
                args.verbose = true;
                break;
            case 'l':
                args.laser = true;
                break;
            case 't':
                args.text = true;
                break;
            case 'o':
                args.odom = true;
                break;
            case 'm':
                args.map = true;
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
                return -1;
                break;
        }
    }

    if (args.filename && args.serialport) {
        cerr << "You can supply a path to a serial dump file OR a path to a serial port, not both" << endl;
        return -1;
    }

    if (!args.filename && !args.serialport) {
        displayUsage();
        return -1;
    }

    p.setGui(!args.cli);

    int verbosity = 0;
    verbosity |= (args.verbose ? parser::VERB_DEBUG : 0)
        | (args.laser ? parser::VERB_LASER : 0)
        | (args.text ? parser::VERB_TEXT : 0)
        | (args.map ? parser::VERB_MAP : 0)
        | (args.odom ? parser::VERB_ODOM : 0);
    p.setVerbosity(verbosity);

    if (args.cli) {
        cout << "Running in command line mode" << endl;
    } else {
        cout << "Running in gui mode" << endl;
    }

    // override sigint (ctrl-c)
    signal(SIGINT, term);

    if (args.serialport) {
        struct termios tty_opt;
        int tty_fd;

        cerr << "Serial ports not yet supported, use at your own risk" << endl;

        cout << "Opening serial port " << args.serialport << endl;
        tty_fd = open(args.serialport, O_RDWR | O_NONBLOCK);
        if (tty_fd < 0) {
            cerr << "Could not open port " << args.serialport << endl;
            return -1;
        }

        memset(&tty_opt, 0, sizeof(tty_opt));
        
        tty_opt.c_cflag = CS8 | CLOCAL | CREAD; // 8N1
        tty_opt.c_iflag = 0;
        tty_opt.c_oflag = 0;
        tty_opt.c_lflag = 0; // noncanonical mode
        tty_opt.c_cc[VMIN] = 1; // one char is enough
        tty_opt.c_cc[VTIME] = 0; // no timer

        cfsetospeed(&tty_opt, B115200); // 115200 baud
        cfsetispeed(&tty_opt, B115200); // 115200 baud

        tcsetattr(tty_fd, TCSANOW, &tty_opt);

        if (write(tty_fd, activation_cmd, strlen(activation_cmd))) {
            while (!done) {
                unsigned char c;
                if (read(tty_fd, &c, 1) > 0) {
                    p.update(c);
                }      
            }
            close(tty_fd);
        } else {
            cerr << "Couldn't write activation command" << endl;
            return -1;
        }

    } else if (args.filename) {
        cout << "Opening input file " << args.filename << endl;
        ifstream file(args.filename, ios::in | ios::binary | ios::ate);
        if (file.is_open()) {
            cout << "Parsing file..." << endl;
            ifstream::pos_type size = file.tellg();
            file.seekg(0, ios::beg);

            // simulating serial input here
            cout << "Iterating through chars" << endl;
            for (int index = 0; index < size && !done; index++) {
                char c;
                file.read(&c, 1); // c now has new char
                p.update(c);
            }
            file.close();

            if (!args.cli) {
                cout << "Ctrl-C to exit" << endl;
                while (!done) {
                    usleep(1000);
                }
            }
        } else {
            cerr << "Could not open file " << args.filename << endl;
            return -1;
        }
    }

    if (args.gifname) {
        cout << "Writing gif to " << args.gifname << endl;
        p.writeMap(args.gifname);
    }
}
