#ifndef PARSER_H_

#include <vector>

namespace parser {
    const static unsigned char HEADER[] = { 0x01, 0x02, 0x03, 0x04 };
    const static unsigned char FOOTER[] = { 0x40, 0x30, 0x20, 0x10 };

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

    /*!
     * Call with new characters to get them parsed
     * @param c character to parse
     */
    void update(char c);

    /*!
     * Checks if there is a header at a given position
     * @param buf a message
     * @param pos the position to check for a header
     * @return true if pos is the beginning of a header
     */
    bool is_header(const std::vector<unsigned char>& buf, int pos);

    /*!
     * Checks if there is a footer ending at a given position
     * @param buf a message
     * @param pos the position to check for a footer
     * @return true if pos is the end of a footer
     */
    bool is_footer(const std::vector<unsigned char>& buf, int pos);

    /*!
     * Processes a message
     * @param msg the message to be processed
     */
    void processMsg(const std::vector<unsigned char>& msg);

    /*!
     * Processes a text message
     * @param msg the message to be processed
     */
    void processText(const std::vector<unsigned char>& msg);

    /*!
     * Processes a map message
     * @param msg the message to be processed
     */
    void processMap(const std::vector<unsigned char>& msg);

    /*!
     * Writes a PGM-formatted map
     * @param filename the file to be written
     */
    void writeMap(const char *filename);

    /*!
     * Processes a laser message
     * @param msg the message to be processed
     */
    void processLaser(const std::vector<unsigned char>& msg);

    /*!
     * Constructs a long, LSB first.
     * @param msg the message buffer
     * @param pos the position in the message buffer
     * @return the constructed long
     */
    long construct_long(const std::vector<unsigned char>& msg, int pos);

    /*!
     * Constructs an int, LSB first.
     * @param msg the message buffer
     * @param pos the position in the message buffer
     * @return the constructed int
     */
    int construct_int(const std::vector<unsigned char>& msg, int pos);
}

#endif /* PARSER_H_ */
