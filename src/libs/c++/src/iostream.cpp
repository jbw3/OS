#include <iostream>
#include "unistd.h"

class posix_write_streambuf : public std::basic_streambuf<char>
{
public:
    posix_write_streambuf() = default;

    virtual ~posix_write_streambuf() = default;

protected:
    int_type overflow(int_type ch = traits_type::eof()) override
    {
        ssize_t rv = write(STDOUT_FILENO, &ch, 1);

        if (rv < 0)
        {
            return traits_type::eof();
        }
        else
        {
            return 0;
        }
    }
};

posix_write_streambuf cout_streambuf;

namespace std
{

ostream cout(&cout_streambuf);

} // namespace std
