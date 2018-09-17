#include <agi/view.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <assert.h>
#include <queue>

namespace agi {

void SetPixel(uint8_t x, uint8_t y, uint8_t color, Cel& cel)
{
    if ((x < cel.width) && (y < cel.height)) {
        size_t offset = (cel.width * y) + x;
        cel.pixels[offset] = color;
    }
}

void ParseCel(Source& source, Cel& cel)
{
    uint8_t w = source.GetU8();
    uint8_t h = source.GetU8();
    uint8_t f = source.GetU8();

    cel.width = w;
    cel.height = h;
    cel.colorKey = f & 0x0f;
    cel.mirrored = (f >> 7) & 0x01;
    cel.mirrorLoop = (f >> 4) & 0x07;

    cel.pixels.resize(cel.width * cel.height);
    std::fill(cel.pixels.begin(), cel.pixels.end(), cel.colorKey);

    size_t x = 0;
    size_t y = 0;
    while(y < cel.height) {
        const uint8_t b = source.GetU8();
        if (b == 0) {
            // skip to the next line
            x = 0;
            ++y;
        }
        else {
            uint8_t count = (b & 0x0f);
            uint8_t color = (b >> 4);
            for(uint8_t i = 0; i < count; ++i) {
                SetPixel(x, y, color, cel);
                ++x;
            }
        }
    }
}

void ParseLoop(Source& source, Loop& loop)
{
    // position is now v + lofs
    const auto v_plus_lofs = source.GetOffset();
    auto numCels = source.GetU8();
    // position is now v + lofs + 1
    std::vector<size_t> offsets(numCels);
    for(auto& offset : offsets) {
        offset = source.GetU16_LE();
    }
    // source is now after the loop header
    loop.cels.resize(numCels);
    for(size_t i = 0; i < numCels; ++i) {
        source.SetOffset(v_plus_lofs + offsets[i]);
        ParseCel(source, loop.cels[i]);
    }
}

void ParseView(Source& source, View& result)
{
    // source is now V
    auto v = source.GetOffset();
    source.GetU8();                             // Unknown, v + 0
    source.GetU8();                             // Unknown, v + 1
    auto loopCount = source.GetU8();            // loop count, v + 2
    auto descriptionPos = source.GetU16_LE();   // description, v + 3/4

    // source is now at "lptr"
    std::vector<size_t> offsets(loopCount);
    for(uint8_t i = 0; i < loopCount; ++i) {
        offsets[i] = source.GetU16_LE();
    }

    // v + lofs = Loop header
    result.loops.resize(loopCount);
    for(size_t i = 0; i < loopCount; ++i) {
        source.SetOffset(v + offsets[i]);   // v + lofs
        ParseLoop(source, result.loops[i]);
    }
}

void ParseViewResource(Source& source, View& result)
{
    // read the magic number
    if (source.GetU16_BE() != 0x1234) {
        throw std::runtime_error("Invalid script magic number.");
    }
    const uint8_t vol = source.GetU8();
    const uint16_t length = source.GetU16_LE();
    ParseView(source, result);
}

} // namespace agi
