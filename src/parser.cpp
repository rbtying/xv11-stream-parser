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

#include "parser.h"
#include <iostream>
#include <fstream>
#include <inttypes.h>
#include <Magick++.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cmath>

using namespace std;
using namespace Magick;
using namespace cv;

const static unsigned char HEADER[] = { 0x01, 0x02, 0x03, 0x04 };
const static unsigned char FOOTER[] = { 0x40, 0x30, 0x20, 0x10 };

parser::parser(const char *name, bool gui, int delayTime) : m_name(name) {
    m_gui_running = gui;
    m_delay_time = delayTime;
    m_verbose = 0;
    if (m_gui_running) {
        namedWindow(name, CV_WINDOW_AUTOSIZE);
        namedWindow("Laser", CV_WINDOW_AUTOSIZE);
        Mat img = Mat::zeros(512, 512, CV_8UC1);
        imshow("Laser", img);
        moveWindow("Laser", 512, 0);
    }
}

parser::~parser() {
    if (m_gui_running) {

    }
}

void parser::setGui(bool gui) {
    m_gui_running = gui;
}

void parser::setVerbosity(int verbose) {
    m_verbose = verbose;
}

void parser::update(char c) {
    m_buf.push_back(c); // store the character

    if (is_footer(m_buf.size() - 1)) { // if end of message
        processMsg();
        m_buf.clear(); // clear the m_buffer
    } else if (is_header(m_buf.size() - 1)) { // if end of header
        m_buf.clear();
        for (int i = 0; i < 4; i++) {
            m_buf.push_back(HEADER[i]);
        }
    } 
}

bool parser::is_header(int pos) {
    // if there are enough unsigned characters
    if (pos - 3 >= 0 && pos < static_cast<int>(m_buf.size())) {
        for (int i = 0; i < 4; i++) {
            if (m_buf[(pos - 3) + i] != HEADER[i]) {
                return false;
            }
        }
        return true;
    } else {
        return false;
    }
}

bool parser::is_footer(int pos) {
    // if there are enough unsigned characters
    if (pos - 3 >= 0 && pos < static_cast<int>(m_buf.size())) {
        for (int i = 0; i < 4; i++) {
            if (m_buf[(pos - 3) + i] != FOOTER[i]) {
                return false;
            }
        }
        return true;
    } else {
        return false;
    }
}

long parser::construct_long(int pos) {
    long tmp = static_cast<int32_t>(m_buf[pos] | m_buf[pos + 1] << 8 | m_buf[pos + 2] << 16 | m_buf[pos + 3] << 24);
    return tmp;
}

int parser::construct_int(int pos) {
    int tmp = static_cast<int16_t>(m_buf[pos] | m_buf[pos + 1] << 8);
    return tmp;
}

void parser::processMsg() {
    // verify header
    if (!is_header(3) && (m_verbose & VERB_DEBUG)) {
        cerr << "ERRROR: Header does not match" << endl;
        return;
    }

    // verify footer
    if (!is_footer(m_buf.size() - 1) && (m_verbose & VERB_DEBUG)) {
        cerr << "ERROR: Footer does not match" << endl;
        return;
    }

    // sequence number (increment by one per message)
    long timestamp = construct_long(TIMESTAMP);
    unsigned int seq = construct_int(SEQUENCE);
    int type = construct_int(TYPE);
    
    if (m_verbose & VERB_DEBUG) {
        cout << seq << " (" << timestamp << "):\ttype: " << type << "\t";
    }

    switch(type) {
        case TEXT:
            processText();
            break;
        case MAP:
            processMap();
            break;
        case LASER:
            processLaser();
            break;
        default:
            /* cout << endl; */
            break;
    }

    if (m_verbose & VERB_DEBUG) {
        cout << endl;
    }
}

void parser::processText() {
    long string_length = construct_long(STR_LEN);
    unsigned char *text_buf = new unsigned char[string_length + 1];

    if (m_verbose & (VERB_TEXT | VERB_DEBUG)) {
        cout << "(text, " << string_length << " bytes): ";
    }

    for (int i = 0; i < string_length; i++) {
        text_buf[i] = m_buf[STR_DATA + i];
    }
    text_buf[string_length] = '\0';

    if (m_verbose & VERB_TEXT) {
        cout << text_buf;
        if (text_buf[string_length - 1] != '\n') {
            cout << endl;
        }
    }

    delete[] text_buf;
}


void parser::processMap() {
    fstream file;
    
    // read existing data
    // edit data with new input
    long size = construct_long(MAP_SIZE);
    long address = construct_long(MAP_ADDR);

    if (m_verbose & VERB_MAP || m_verbose & VERB_DEBUG) {
        cout << "\t(map, " << size << " bytes at 0x" << hex <<  address << dec << ")" << endl;
    }

    copy(m_buf.begin() + MAP_DATA, m_buf.begin() + MAP_DATA + size, m_img + address);

    if (m_gui_running) {
        Mat img(256, 256, CV_8UC1);
        copy(m_img, m_img + 65536, img.data);
        imshow(m_name, img);
        waitKey(m_delay_time);
    }

    Image temp(Geometry(256, 256), Color(0, 0, 0, 0));

    temp.magick("gray");
    temp.read(256, 256, "I", CharPixel, m_img);
    temp.animationDelay(1);
    
    m_images.push_back(temp);
}

void parser::writeMap(const char *filename) {
    Magick::writeImages(m_images.begin(), m_images.end(), filename); 
}

void parser::processLaser() {
    long index = construct_long(LSR_INDEX);
    
    if (m_verbose & VERB_LASER) {
        cout << "(laser, " << index << " deg): " ;
    }

    for (int i = 0; i < 90; i++) {
        laser_unit *pt = &m_laser[index + i];
        pt->x = construct_int(LSR_DATA + 4 * i);
        pt->y = construct_int(LSR_DATA + 4 * i + 2);
        if (m_verbose & VERB_LASER) {
            if (m_verbose & VERB_DEBUG) {
                if (pt->x < 32767 && pt->y < 32767) {
                    cout << "(" << pt->x << ", " << pt->y << ")" << endl;
                } else {
                    cout << "Out of range" << endl;
                }
            }
        }
    }
    if (m_gui_running && index == 270) {
        Mat img = Mat::zeros(512, 512, CV_8UC1);
        for (int i = 0; i < 360; i++) {
            if (m_laser[i].x < 32767 && m_laser[i].y < 32767) { 
                // point is in range 
                int x = (m_laser[i].x / 2) + 256;
                int y = (m_laser[i].y / 2) + 256;

                if (x >= 0 && x < 512 && y >= 0 && y < 512) {
                    img.data[x + y * 512] = 0xff;
                }
            }
        }
        imshow("Laser", img);
        waitKey(m_delay_time);
    }
    
    if (m_verbose & VERB_LASER) {
        cout << endl;
    }
}
