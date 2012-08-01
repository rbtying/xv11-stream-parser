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

#ifndef PARSER_H_

#include <vector>
#include <Magick++.h>
#include <opencv2/core/core.hpp>

using std::vector;
using std::string;
using Magick::Image;
using cv::Mat;
using cv::Point;

class parser {
/* public functions */
public:
    /*!
     * Constructs a parser object
     * @param name Name of the window
     * @param gui whether or not to show a GUI
     */
    parser(const char *name, bool gui = true, int delayTime = 1);

    /*!
     * Destructs a parser object
     */
    virtual ~parser();

    /*!
     * Determines whether or not to use a gui
     * @param gui use a gui
     */
    void setGui(bool gui = true);

    /*!
     * Sets the verbosity of the program
     * @param verbose bitmask for verbosity
     */
    void setVerbosity(int verbose);

    /*!
     * Call with new characters to get them parsed
     * @param c character to parse
     */
    void update(char c);

    /*!
     * Writes a gif map animation
     * @param filename the file to be written
     */
    void writeMap(const char *filename);

    /*!
     * Writes a gif laser animation
     * @param filename the file to be written
     */
    void writeAnim(const char *filename);

/* private functions */
private:
    /*!
     * Checks if there is a header at a given position
     * @param pos the position to check for a header
     * @return true if pos is the beginning of a header
     */
    bool is_header(int pos);

    /*!
     * Checks if there is a footer ending at a given position
     * @param pos the position to check for a footer
     * @return true if pos is the end of a footer
     */
    bool is_footer(int pos);

    /*!
     * Processes a message
     */
    void processMsg();

    /*!
     * Processes an odometry message
     */
    void processOdom();

    /*!
     * Processes a text message
     */
    void processText();

    /*!
     * Processes a map message
     */
    void processMap();

    /*!
     * Processes a laser message
     */
    void processLaser();

    /*!
     * Constructs a long, LSB first.
     * @param pos the position in the message buffer
     * @return the constructed long
     */
    long construct_long(int pos);

    /*!
     * Constructs an int, LSB first.
     * @param pos the position in the message buffer
     * @return the constructed int
     */
    int construct_int(int pos);

    /*!
     * Checks if a point is in bounds
     * @param mat the matrix to check with
     * @param x the x-coordinate
     * @param y the y-coordinate
     * @return true if in bounds
     */
    bool inBounds(const Mat& mat, int x, int y);

    /*!
     * Shifts and scales a point to fit into the matrix
     * @param mat the matrix to fit into
     * @param point the point to shift/scale
     * @param min minimum values of original point
     * @param max maximum values of original point
     * @return the scaled point, or (-32767, -32767) if point is out of range.
     */
    Point convertPoint(const Mat& mat, const Point& pt, const Point& min, const Point& max);

    /*!
     * Finds the intersection of the line p1p2 and p3p4
     * @param p1 point 1 of one line
     * @param p2 point 2 of the first line
     * @param p3 point 1 of second line
     * @param p4 point 2 of the second line
     * @return intersection of the points, or (-32767, -32767) if there is no
     * intersection
     */
    Point intersection(const Point& p1, const Point& p2, const Point& p3, const Point& p4);

    enum MSG_PKT {
        HEAD        = 0x00,
        TYPE        = 0x04,
        SEQUENCE    = 0x06,
        TIMESTAMP   = 0x08,
        /* text messages */
        STR_LEN     = 0x0c,
        STR_DATA    = 0x10,
        /* laser */
        LSR_INDEX   = 0x10,
        LSR_DATA    = 0x14,
        /* map */
        MAP_SIZE    = 0x0c,
        MAP_ADDR    = 0x10,
        MAP_DATA    = 0x18,
        /* odom */
    };

    enum MSG_TYPES {
        POSITION    = 0x01,
        LASER       = 0x05,
        MAP         = 0x09,
        TEXT        = 0x11,
    };

    char m_img[65536];
    vector<Image> m_images;
    vector<Image> m_laser_images;
    vector<unsigned char> m_buf;
    string m_name;

    bool m_gui_running;

    struct laser_unit {
        Point pt;
        bool valid;
    };

    struct laser_unit m_laser[360];

    Point m_center;

    struct odom_data {
        double count;
        double speed;
    };

    struct odom_data left;
    struct odom_data right;

public:
    enum VERBOSITY_LVL {
        VERB_DEBUG  = (1 << 0),
        VERB_TEXT   = (1 << 1),
        VERB_LASER  = (1 << 2),
        VERB_MAP    = (1 << 3),
        VERB_ODOM   = (1 << 4),
    };
private:

    int m_verbose;
    int m_delay_time;
};

#endif /* PARSER_H_ */
