#ifndef _STRINGUTILS_H
#define _STRINGUTILS_H

#include <string.h>

char digitToChar(char digit, bool uppercase);

template<typename T>
int signedIntToString(T num, char* str, int base, bool uppercase)
{
    // need sizeof(num) * 8 chars for max signed number,
    // 1 char for possible negative sign,
    // and 1 char for null
    constexpr int BUFF_SIZE = sizeof(num) * 8 + 2;
    char buff[BUFF_SIZE];
    buff[BUFF_SIZE - 1] = '\0';

    int idx = BUFF_SIZE - 1;
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

    return (BUFF_SIZE - 1) - idx;
}

template<typename T>
int unsignedIntToString(T num, char* str, int base, bool uppercase)
{
    // need sizeof(num) * 8 chars for max signed number
    // and 1 char for null
    constexpr int BUFF_SIZE = sizeof(num) * 8 + 1;
    char buff[BUFF_SIZE];
    buff[BUFF_SIZE - 1] = '\0';

    int idx = BUFF_SIZE - 1;
    do
    {
        char digit = num % base;
        digit = digitToChar(digit, uppercase);
        buff[--idx] = digit;
        num /= base;
    } while (num > 0);

    strcpy(str, buff + idx);

    return (BUFF_SIZE - 1) - idx;
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
