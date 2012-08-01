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

bool parser::inBounds(const Mat& mat, int x, int y) {
    return x >= 0 && y >= 0 && x < mat.cols && y < mat.rows;
}

Point parser::convertPoint(const Mat& mat, const Point& pt, const Point& min, const Point& max) {
    Point out;

    double xScale = (max.x - min.x) * 1.0 / mat.cols;
    double yScale = (max.y - min.y) * 1.0 / mat.rows;

    out.x = (pt.x - (max.x + min.x) / xScale + mat.cols) / 2;
    out.y = (pt.y - (max.y + min.y) / yScale + mat.cols) / 2;

    if (inBounds(mat, out.x, out.y)) {
        return out;
    } else {
        return Point(-32767, -32767);
    }
}

Point parser::intersection(const Point& p1, const Point& p2, const Point& p3, const Point& p4) {
    double m1 = (p2.y - p1.y) * 1.0 / (p2.x - p1.x);
    double m2 = (p4.y - p3.y) * 1.0 / (p4.x - p3.x);

    if (m1 == m2) {
        return Point(-32767, -32767);
    }

    double x = (p1.y - p3.y + p3.x * m2 - p1.x * m1) / (m2 - m1);
    double y = m1 * (x - p1.x) + p1.y;
   
    return Point(x, y);
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
    unsigned long timestamp = construct_long(TIMESTAMP);
    unsigned int seq = construct_int(SEQUENCE);
    int type = construct_int(TYPE);
    
    if (m_verbose & VERB_DEBUG) {
        cout << seq << " (" << timestamp << ")\ttype: " << type << "\t\t";
    }

    switch(type) {
        case POSITION:
            processOdom();
            break;
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

void parser::processOdom() {
    if (m_verbose & (VERB_ODOM)) {
        cout << "(odom, " << (m_buf.size() - 0x0c - 4) << " bytes)\t";
        // for (int i = 0; i < m_buf.size() - 0x0c - 4; i++) {
        //     cout << "0x" << hex << static_cast<int>(m_buf[0x0c + i]) << dec << ", ";
        // }
        // cout << endl;
        // odom_data pleft, pright;
        // pleft.count = left.count;
        // pleft.speed = left.speed;
        // pright.count = right.count;
        // pright.speed = right.speed;
        // 
        left.count = construct_long(0x0c); // maybe encoder counts?
        right.count = construct_long(0x10); // maybe encoder counts?
        long noclue = construct_long(0x18); // constant at 32000 no clue what this is
        left.speed = construct_int(0x14) * 0.001; // maybe encoder count rate?
        right.speed = construct_int(0x16) * 0.001; // maybe encoder count rate?
        cout << left.count << "\t" << right.count << "\t" << m_center.x << "\t" << m_center.y;
        // /* cout << ldata[0] << "\t" << ldata[1] << "\t" << idata[0] << "\t" << idata[1] << "\t" << ldata[2]; */

        // cout << left.count - pleft.count << "\t" << left.speed << "\t" << right.count - pright.count << "\t" << right.speed;
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

    if (m_verbose & (VERB_MAP | VERB_DEBUG)) {
        cout << "(map, " << size << " bytes at 0x" << hex <<  address << dec << ")" << endl;
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

void parser::writeAnim(const char *filename) {
    Magick::writeImages(m_laser_images.begin(), m_laser_images.end(), filename); 
}

void parser::processLaser() {
    long index = construct_long(LSR_INDEX);
    
    if (m_verbose & (VERB_LASER | VERB_DEBUG)) {
        cout << "(laser, " << index << " deg)\t";
    }

    for (int i = 0; i < 90; i++) {
        laser_unit *u = &m_laser[index + i];
        u->pt.x = construct_int(LSR_DATA + 4 * i);
        u->pt.y = construct_int(LSR_DATA + 4 * i + 2);
        u->valid = abs(u->pt.x) < 512 && abs(u->pt.y) < 512;
        if (m_verbose & VERB_LASER) {
            if (m_verbose & VERB_DEBUG) {
                if (u->valid) {
                    cout << "(" << u->pt.x << ", " << u->pt.y << ")" << endl;
                } else {
                    cout << "Out of range" << endl;
                }
            }
        }
    }
    if (index == 270) {
        Mat img = Mat::zeros(512, 512, CV_8UC3);
        Point min(-512, -512);
        Point max(512, 512);
        for (int i = 0; i < 360; i++) {
            if (m_laser[i].valid) { 
                // point is in range 
                Point sc = convertPoint(img, m_laser[i].pt, min, max);

                if (inBounds(img, sc.x, sc.y)) {
                    for (int j = -1; j <= 1; j++) {
                        for (int k = -1; k <= 1; k++) {
                            if (inBounds(img, sc.x + j, sc.y + k) && (j != k)) {
                                img.at<Vec3b>(sc.y + k, sc.x + j)[2] = 0xff;
                            }
                        }
                    }
                    img.at<Vec3b>(sc.y, sc.x)[2] = 0xff;

                }
            }
        }

        vector<Point> intersections;

        for (int i = 0; i < 180; i++) {
            // draw lines
            // only need to draw for points 0-179 b/c they're drawn to
            // opposing point. Also draw less lines to make it easier to
            // see
            Point sc[4];
            bool invalid = false;
            for (int j = 0; j < 4; j++) {
                if (m_laser[i + j * 90].valid) {
                    sc[j] = convertPoint(img, m_laser[i + j * 90].pt, min, max);
                } else {
                    invalid = true;
                }
            }

            if (invalid) {
                continue;
            }

            for (int j = 0; j < 2; j++) {
                if (inBounds(img, sc[j].x, sc[j].y) && inBounds(img, sc[j + 2].x, sc[j + 2].y)) {
                    line(img, sc[j], sc[j + 2], CV_RGB(0, 255, 0));
                }
            }
            // poor man's method of finding the center
            intersections.push_back(intersection(sc[0], sc[2], sc[1], sc[3]));
        }

        // average the various intersections
        Point center;
        int points = 0;
        if (intersections.size()) {
            for (unsigned int i = 0; i < intersections.size(); i++) {
                if (inBounds(img, intersections[i].x, intersections[i].y)) {
                    center.x += intersections[i].x;
                    center.y += intersections[i].y;
                    ++points;
                }
            }

            if (points) {
                center.x /= points;
                center.y /= points;
                
                // this is rather suboptimal =(
                if (m_verbose & VERB_LASER) {
                    cout << "Intersection: " << center;
                }

                for (int j = -5; j <= 5; j++) {
                    for (int k = -5; k <= 5; k++) {
                        if (inBounds(img, center.x + j, center.y + k)) {
                            img.at<Vec3b>(center.y + k, center.x + j)[0] = 0xff;
                        }
                    }
                }
                if (inBounds(img, center.x, center.y)) {
                    m_center = center;
                }
            }
        }

        Image temp(img.cols, img.rows, "BGR", CharPixel, reinterpret_cast<char*>(img.data));
        temp.animationDelay(1);
    
        m_laser_images.push_back(temp);

        if (m_gui_running) {
            imshow("Laser", img);
            waitKey(m_delay_time);
        }
    }
    
    if (m_verbose & VERB_LASER) {
        cout << endl;
    }
}
