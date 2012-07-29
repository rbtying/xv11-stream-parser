#ifndef PARSER_H_

#include <vector>
#include <Magick++.h>

using std::vector;
using Magick::Image;

class parser {
/* public functions */
public:
    /*!
     * Constructs a parser object
     */
    parser();

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
};

#endif /* PARSER_H_ */
