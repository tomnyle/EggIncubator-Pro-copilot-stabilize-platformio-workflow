#pragma once

#include <cstdint>
#include <cstdio>
#include <string>

class String
{
public:
    String() = default;
    String(const char *value) : text(value) {}
    String(float value, unsigned int precision)
    {
        char buffer[48];
        std::snprintf(buffer, sizeof(buffer), "%.*f", precision, value);
        text = buffer;
    }
    String(int value) : text(std::to_string(value)) {}
    const char *c_str() const { return text.c_str(); }
    friend String operator+(const String &a, const String &b)
    {
        return String((a.text + b.text).c_str());
    }
private:
    std::string text;
};

struct FakeSerial
{
    std::string input;
    void begin(uint32_t) {}
    int available() { return static_cast<int>(input.size()); }
    int read()
    {
        const char value = input.front();
        input.erase(0, 1);
        return value;
    }
    template <typename T> void print(const T &) {}
    template <typename T> void print(const T &, int) {}
    template <typename T> void println(const T &) {}
    void println() {}
    template <typename... Args>
    void printf(const char *, Args...) {}
};

extern FakeSerial Serial;
extern uint32_t fakeMillis;
extern int pinValues[40];

inline uint32_t millis() { return fakeMillis; }
inline void delay(uint32_t ms) { fakeMillis += ms; }
inline void digitalWrite(int pin, int value) { pinValues[pin] = value; }
template <typename T>
T constrain(T value, T minimum, T maximum)
{
    return value < minimum ? minimum : value > maximum ? maximum : value;
}

#define HIGH 1
#define LOW 0
