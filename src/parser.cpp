#include "parser.h"
#include <iostream>
#include <fstream>
#include <inttypes.h>
#include <Magick++.h>
#include <cmath>

using namespace std;
using namespace Magick;

vector<unsigned char> buf;

void parser::update(char c) {
    buf.push_back(c); // store the character

    if (parser::is_footer(buf, buf.size() - 1)) { // if end of message
        parser::processMsg(buf);
        buf.clear(); // clear the buffer
    } else if (parser::is_header(buf, buf.size() - 1)) { // if end of header
        buf.clear();
        for (int i = 0; i < 4; i++) {
            buf.push_back(parser::HEADER[i]);
        }
    } 
}

bool parser::is_header(const std::vector<unsigned char>& buf, int pos) {
    // if there are enough unsigned characters
    if (pos - 3 >= 0 && pos < static_cast<int>(buf.size())) {
        for (int i = 0; i < 4; i++) {
            if (buf[(pos - 3) + i] != HEADER[i]) {
                return false;
            }
        }
        return true;
    } else {
        return false;
    }
}

bool parser::is_footer(const std::vector<unsigned char>& buf, int pos) {
    // if there are enough unsigned characters
    if (pos - 3 >= 0 && pos < static_cast<int>(buf.size())) {
        for (int i = 0; i < 4; i++) {
            if (buf[(pos - 3) + i] != FOOTER[i]) {
                return false;
            }
        }
        return true;
    } else {
        return false;
    }
}

long parser::construct_long(const std::vector<unsigned char>& msg, int pos) {
    long tmp = static_cast<int32_t>(msg[pos] | msg[pos + 1] << 8 | msg[pos + 2] << 16 | msg[pos + 3] << 24);
    return tmp;
}

int parser::construct_int(const std::vector<unsigned char>& msg, int pos) {
    int tmp = static_cast<int16_t>(msg[pos] | msg[pos + 1] << 8);
    return tmp;
}

void parser::processMsg(const std::vector<unsigned char>& msg) {
    // verify header
    if (!is_header(msg, 3)) {
        cerr << "ERRROR: Header does not match" << endl;
        return;
    }

    // verify footer
    if (!is_footer(msg, msg.size() - 1)) {
        cerr << "ERROR: Footer does not match" << endl;
        return;
    }

    // sequence number (increment by one per message)
    long timestamp = construct_long(msg, TIMESTAMP);
    int seq = construct_int(msg, SEQUENCE);
    int type = construct_int(msg, TYPE);
    
    switch(type) {
        case TEXT:
        case LASER:
        case MAP:
            cout << seq << " (" << timestamp << "):\ttype: " << type << "\t";
            break;
    }
    switch(type) {
        case TEXT:
            processText(msg);
            break;
        case MAP:
            processMap(msg);
            break;
        case LASER:
            processLaser(msg);
            break;
        default:
            /* cout << endl; */
            break;
    }
}

void parser::processText(const std::vector<unsigned char>& msg) {
    long string_length = construct_long(msg, STR_LEN);
    cout << "(text, " << string_length << " bytes): ";
    unsigned char *buf = new unsigned char[string_length + 1];

    for (int i = 0; i < string_length; i++) {
        buf[i] = msg[STR_DATA + i];
    }
    buf[string_length] = '\0';
    cout << buf << endl;

    delete[] buf;
}

char img[65536];
vector<Image> images;
int counter = 0;

void parser::processMap(const std::vector<unsigned char>& msg) {
    fstream file;
    
    // read existing data
    // edit data with new input
    long size = construct_long(msg, MAP_SIZE);
    long address = construct_long(msg, MAP_ADDR);
    copy(msg.begin() + MAP_DATA, msg.begin() + MAP_DATA + size, img + address);

    Image temp(Geometry(256, 256), Color(0, 0, 0, 0));

    temp.magick("gray");
    temp.read(256, 256, "I", CharPixel, img);
    temp.animationDelay(1);
    
    images.push_back(temp);
}

void parser::writeMap(const char *filename) {
    Magick::writeImages(images.begin(), images.end(), filename); 
}

void parser::processLaser(const std::vector<unsigned char>& msg) {
    long index = construct_long(msg, LSR_INDEX);
    cout << "(laser, " << index << " deg): " ;
    for (int i = 0; i < 90; i++) {
        double x = construct_int(msg, LSR_DATA + 4 * i);
        double y = construct_int(msg, LSR_DATA + 4 * i + 2);
        double dist = sqrt(x * x + y * y);
        cout << dist << ", ";
    }
    cout << endl;
}
