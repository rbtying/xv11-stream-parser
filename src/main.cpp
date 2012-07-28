#include <iostream>
#include <fstream>
#include <vector>
#include "parser.h"

using namespace std;

int main (int argc, char** argv) {

    if (argc > 1) {
        cout << "Opening input file " << argv[1] << endl;
        ifstream file(argv[1], ios::in | ios::binary | ios::ate);
        if (file.is_open()) {
            cout << "Parsing file..." << endl;
            ifstream::pos_type size = file.tellg();
            file.seekg(0, ios::beg);

            // simulating serial input here
            cout << "Iterating through chars" << endl;
            for (int index = 0; index < size; index++) {
                char c;
                file.read(&c, 1); // c now has new char
                parser::update(c);
            }
            file.close();

            if (argc > 2) {
                parser::writeMap(argv[2]);
            } else {
                cerr << "Supply second parameter if you want the gif" << endl;
            }
        } else {
            cerr << "Could not open file" << endl;
        }
    } else {
        cerr << "No input file provided!" << endl;
    }

}
