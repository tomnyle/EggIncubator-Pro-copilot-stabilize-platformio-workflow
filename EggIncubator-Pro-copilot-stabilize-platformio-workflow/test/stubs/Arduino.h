#ifndef TEST_ARDUINO_H
#define TEST_ARDUINO_H

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>

class String
{
public:
    String(const char* value) : value_(value) {}

    void trim()
    {
        const auto first = value_.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) { value_.clear(); return; }
        value_ = value_.substr(first, value_.find_last_not_of(" \t\r\n") - first + 1);
    }

    void toUpperCase()
    {
        std::transform(value_.begin(), value_.end(), value_.begin(),
                       [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    }

    bool operator==(const char* other) const { return value_ == other; }

private:
    std::string value_;
};

#endif
