#include "serial_line.h"
#include <cassert>
#include <cstring>

int main()
{
    SerialLine input;
    for (const char* c = "PROFILE DUCK"; *c; ++c)
        assert(input.feed(*c) == SerialLine::Result::None);
    assert(input.feed('\r') == SerialLine::Result::Ready);
    assert(std::strcmp(input.line(), "PROFILE DUCK") == 0);
    assert(input.feed('\n') == SerialLine::Result::None);

    for (int i = 0; i < 32; ++i)
        assert(input.feed('X') == SerialLine::Result::None);
    assert(input.feed('\n') == SerialLine::Result::Invalid);
    assert(input.feed('A') == SerialLine::Result::None);
    assert(input.feed('\n') == SerialLine::Result::Ready);
    assert(std::strcmp(input.line(), "A") == 0);

    assert(input.feed('A') == SerialLine::Result::None);
    assert(input.feed('\0') == SerialLine::Result::None);
    assert(input.feed('\n') == SerialLine::Result::Invalid);
}
