#ifndef STREAM_H_
#define STREAM_H_

#include <stdint.h>
#include <unistd.h>

class Stream
{
public:
    /**
     * @brief Whether this stream supports reading.
     */
    virtual bool canRead() const = 0;

    /**
     * @brief Whether this stream supports writing.
     */
    virtual bool canWrite() const = 0;

    /**
     * @brief Read from the stream.
     */
    virtual ssize_t read(uint8_t* buff, size_t nbyte) = 0;

    /**
     * @brief Write to the stream.
     */
    virtual ssize_t write(const uint8_t* buff, size_t nbyte) = 0;
};

#endif // STREAM_H_
