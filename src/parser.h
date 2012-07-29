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

using std::vector;
using std::string;
using Magick::Image;

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
     * Call with new characters to get them parsed
     * @param c character to parse
     */
    void update(char c);

    /*!
     * Writes a PGM-formatted map
     * @param filename the file to be written
     */
    void writeMap(const char *filename);

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
    };

    enum MSG_TYPES {
        LASER = 0x05,
        MAP = 0x09,
        TEXT = 0x11,
    };

    char m_img[65536];
    vector<Image> m_images;
    vector<unsigned char> m_buf;
    string m_name;

    bool m_gui_running;
    int m_delay_time;
};

#endif /* PARSER_H_ */
