#ifndef _STRINGUTILS_H
#define _STRINGUTILS_H

#include <string.h>

/// The maximum number of characters needed to represent an integer of
/// type T. The maximum number of characters are needed when the number
/// is printed in base 2. In this case, we need one character per bit
/// which is the integer size * 8. We also need one character for a
/// possible negative sign and one character for the terminating NULL.
template<typename T>
constexpr size_t MAX_INT_CHARS = sizeof(T) * 8 + 2;

char digitToChar(char digit, bool uppercase);

template<typename T>
size_t signedIntToString(T num, char* str, int base, bool uppercase)
{
    char buff[MAX_INT_CHARS<T>];
    buff[MAX_INT_CHARS<T> - 1] = '\0';

    size_t idx = MAX_INT_CHARS<T> - 1;
    bool negative = true;
    if (num >= 0)
    {
        negative = false;
        num = -num;
    }

    do
    {
        char digit = -(num % base);
        digit = digitToChar(digit, uppercase);
        buff[--idx] = digit;
        num /= base;
    } while (num < 0);

    if (negative)
    {
        buff[--idx] = '-';
    }

    strcpy(str, buff + idx);

    return (MAX_INT_CHARS<T> - 1) - idx;
}

template<typename T>
size_t unsignedIntToString(T num, char* str, int base, bool uppercase)
{
    char buff[MAX_INT_CHARS<T>];
    buff[MAX_INT_CHARS<T> - 1] = '\0';

    size_t idx = MAX_INT_CHARS<T> - 1;
    do
    {
        char digit = num % base;
        digit = digitToChar(digit, uppercase);
        buff[--idx] = digit;
        num /= base;
    } while (num > 0);

    strcpy(str, buff + idx);

    return (MAX_INT_CHARS<T> - 1) - idx;
}

template<typename T>
const T FLOAT_MAX_ORDER = 1;

/// @todo temporary max order
template<>
const float FLOAT_MAX_ORDER<float> = 1e20f;

/// @todo temporary max order
template<>
const double FLOAT_MAX_ORDER<double> = 1e100;

/// @todo temporary max order
template<>
const long double FLOAT_MAX_ORDER<long double> = 1e200l;

/// @todo handle infinity and NaN
template<typename T>
void floatToFixedString(T num, char* str, unsigned int precision)
{
    int idx = 0;

    if (num < static_cast<T>(0))
    {
        str[idx++] = '-';
        num = -num;
    }

    T order = FLOAT_MAX_ORDER<T>;

    bool outputPoint = false;
    bool outputZeros = false;
    unsigned int fractionDigits = 0;
    while (fractionDigits < precision)
    {
        /// @todo Check order/10. If > 7, subtract 1 from that order
        T result = num / order;
        result += static_cast<T>(0.001); /// @todo this sometimes causes incorrect behavior (e.g. 999.999)
        int digit = static_cast<int>(result);

        if (outputZeros || digit != 0)
        {
            char ch = '0' + digit;
            str[idx++] = ch;
            outputZeros = true;
        }

        if (outputPoint)
        {
            ++fractionDigits;
        }

        T sub = digit * order;
        num -= sub;
        if (num < static_cast<T>(0))
        {
            num = static_cast<T>(0);
        }

        order /= static_cast<T>(10);
        if (!outputPoint && order < static_cast<T>(1))
        {
            if (!outputZeros)
            {
                str[idx++] = '0';
                outputZeros = true;
            }
            str[idx++] = '.';
            outputPoint = true;
        }
    }

    str[idx] = '\0';
}

#endif // _STRINGUTILS_H
