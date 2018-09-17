#include <agi/picture.h>
#include <agi/source.h>
#include <assert.h>
#include <iostream>

namespace agi {

void DrawPictureResource(
    array_view<uint8_t> volume,
    size_t offset,
    Framebuffer& framebuffer)    
{
    // starts with a 5 byte header
    Source source(volume.data(), volume.size(), offset);
    // read the magic number
    if (source.GetU16_BE() != 0x1234) {
        throw std::runtime_error("Invalid script magic number.");
    }
    const uint8_t vol = source.GetU8();
    const uint16_t length = source.GetU16_LE();
    // now create a sub-source for the picture data
    auto pictureSource = source.SubSource(length);
    // now draw the picture from this source
    DrawPicture(pictureSource, framebuffer);
}

void DrawPicture(Source& source, Framebuffer& framebuffer)
{
    std::vector<uint8_t> points;
    while(!source.empty()) {
        auto cmd = source.GetU8();
        switch(cmd) {
        case 0xF0:
            // Change picture colour and enable picture draw.
            framebuffer.SetPictureColor(source.GetU8());
            break;
        case 0xF1:
            framebuffer.DisablePictureDraw();
            break;
        case 0xF2:
            framebuffer.SetPriorityColor(source.GetU8());
            break;
        case 0xF3:
            framebuffer.DisablePriorityDraw();
            break;
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
        case 0xF8:
            points.clear();
            // add the points until 0xF0 or above is encountered
            while(source.Peek() < 0xF0) {
                points.push_back(source.GetU8());
            }

            switch(cmd) {
            case 0xF4: framebuffer.DrawYCorner(points); break;
            case 0xF5: framebuffer.DrawXCorner(points); break;
            case 0xF6: framebuffer.AbsoluteLine(points); break;
            case 0xF7: framebuffer.RelativeLine(points); break;
            case 0xF8:
                for(size_t i = 0; i < points.size(); i += 2) {
                    framebuffer.Fill(points[i], points[i + 1]);    
                }
                break;
            default:
                break;
            }
            break;
        case 0xF9:
            assert(false);
            break;
        case 0xFF:
            return;
        default:
            assert(false);
            break;
        }
    }   
}

} // namespace agi
