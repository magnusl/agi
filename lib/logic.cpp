#include <agi/logic.h>
#include <iostream>

namespace agi {

void ProcessCondition(Source& source)
{
    auto code = source.GetU8();
    if (code == 0xFC) {
        // OR
    }
    else if (code == 0xFF) {
        // terminates the condition
    }
}

void ProcessIfStatement(Source& source)
{
    // processes the condition, until 0xFF is encountered
    ProcessCondition(source);
    // read the length
    auto length = source.GetU16_LE();

    // process commands

}

void ProcessLogicScript(Source& source)
{
    if (source.GetU16_BE() != 0x1234) {
        throw std::runtime_error("Invalid script magic number.");
    }
    const uint8_t vol = source.GetU8();
    const uint16_t scriptLength = source.GetU16_LE();
    const uint16_t textOffset = source.GetU16_LE();

    while(1) {
        auto code = source.GetU8();
        if (code == 0xFF) { // if-statement
            ProcessIfStatement(source);
        }
    }
}

} // namespace agi
