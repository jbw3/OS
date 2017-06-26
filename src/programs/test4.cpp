#include <iostream>
#include <streambuf>
#include "string.h"
#include "unistd.h"

using std::cout;

void print(const char* str)
{
    write(STDOUT_FILENO, str, strlen(str));
}

int main()
{
    const char str[] = "test4\n";

    static_assert(sizeof(std::char_traits<char>::char_type) == 1);
    static_assert(std::char_traits<char>::eof() == -1);
    static_assert(sizeof(wchar_t) == 4);

    print(str);
    cout.put('!');

    return 0;
}
