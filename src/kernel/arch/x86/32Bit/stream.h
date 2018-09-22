#ifndef STREAM_H_
#define STREAM_H_

#include <stdint.h>
#include <unistd.h>

/**
 * @brief An abstract base class for reading and/or writing data.
 */
class Stream
{
public:
    /**
     * @brief Whether this stream supports reading.
     * @return true if the stream supports reading.
     * @return false if the stream does not support reading.
     */
    virtual bool canRead() const = 0;

    /**
     * @brief Whether this stream supports writing.
     * @return true if the stream supports writing.
     * @return false if the stream does not support writing.
     */
    virtual bool canWrite() const = 0;

    /**
     * @brief Read from the stream.
     */
    virtual ssize_t read(uint8_t* buff, size_t nbyte) = 0;

    /**
     * @brief Write to the stream.
     * @details This is a non-blocking call.
     * @param buff The data to write.
     * @param nbyte The number of bytes in the buffer.
     * @return The number of bytes written if successful, or a number less than 0 if an error occurred.
     */
    virtual ssize_t write(const uint8_t* buff, size_t nbyte) = 0;

    /**
     * @brief Write to the stream.
     * @param buff The data to write.
     * @param nbyte The number of bytes in the buffer.
     * @param block Whether to block until all data has been written.
     * @return The number of bytes written if successful, or a number less than 0 if an error occurred.
     */
    ssize_t write(const uint8_t* buff, size_t nbyte, bool block);

    /**
     * @brief Flush any internal stream buffers.
     */
    virtual void flush() = 0;

    /**
     * @brief Close the stream.
     */
    virtual void close() = 0;
};

#endif // STREAM_H_
