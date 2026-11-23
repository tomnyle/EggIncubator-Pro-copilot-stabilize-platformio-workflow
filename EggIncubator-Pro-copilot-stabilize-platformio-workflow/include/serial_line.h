#ifndef SERIAL_LINE_H
#define SERIAL_LINE_H

#include <stddef.h>

class SerialLine
{
public:
    enum class Result { None, Ready, Invalid };

    Result feed(char c)
    {
        if (c == '\r' || c == '\n')
        {
            if (invalid)
            {
                invalid = false;
                length = 0;
                return Result::Invalid;
            }
            if (length == 0) return Result::None;
            buffer[length] = '\0';
            length = 0;
            return Result::Ready;
        }

        if (c == '\0' || length == sizeof(buffer) - 1) invalid = true;
        if (!invalid) buffer[length++] = c;
        return Result::None;
    }

    const char* line() const { return buffer; }

private:
    char buffer[32] = {};
    size_t length = 0;
    bool invalid = false;
};

#endif
