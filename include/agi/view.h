#pragma once

#include <agi/source.h>
#include <stdint.h>
#include <vector>

namespace agi {

struct Cel
{
    uint8_t width;
    uint8_t height;
    uint8_t colorKey    : 4;
    uint8_t mirrored    : 1;
    uint8_t mirrorLoop  : 3;
    std::vector<uint8_t> pixels;    // the pixel data
};

struct Loop
{
    std::vector<Cel> cels;   // the cels in the loop
};

struct View
{
    std::vector<Loop> loops; // the loops in the view
};

/**
 * \brief   Parses an loop
 */
void ParseView(Source& source, View& result);

void ParseViewResource(Source& source, View& result);
    
} // namespace agi
