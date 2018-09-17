#pragma once

#include <vector>
#include <stdint.h>
#include <array>
#include <assert.h>
#include <iostream>

namespace agi {

using Points = std::vector<uint8_t>;

enum Color {
    kBlack          = 0,
    kBlue           = 1,
    kGreen          = 2,
    kCyan           = 3,
    kRed            = 4,
    kMagenta        = 5,
    kBrown          = 6,
    kLightgrey      = 7,
    kDarkGrey       = 8,
    kLightBlue      = 9,
    kLightGreen     = 10,
    kLightCyan      = 11,
    kLightRed       = 12,
    kLightMagenta   = 13,
    kYellow         = 14,
    kWhite          = 15
};

/**
 * \class   Framebuffer
 */
class Framebuffer
{
public:
    enum {
        kWidth      = 160,
        kHeight     = 200,
        kPixelPitch = 320
    };

    Framebuffer();
    void Clear();
    void SetPictureColor(uint8_t);
    void DisablePictureDraw();
    void SetPriorityColor(uint8_t);
    void DisablePriorityDraw();
    void DrawYCorner(const Points&);
    void DrawXCorner(const Points&);
    void AbsoluteLine(const Points&);
    void RelativeLine(const Points&);
    void Fill(uint8_t x, uint8_t y);
    void Display(uint8_t row, uint8_t col, const char* text);
    void ClearLines(uint8_t start, uint8_t stop, uint8_t color);

    const std::array<uint8_t, 64000>& GetPictureBuffer() const noexcept {
        return picture_;
    }

    const std::array<uint8_t, 32000>& GetPriorityBuffer() const noexcept {
        return priority_;
    }

    /**
     * \brief   Sets a pixel if the new pixel has higher priority
     */
    inline void SetPixelIfHigherPriority(uint8_t x, uint8_t y, uint8_t color, uint8_t priority) {
        if ((x < kWidth) && (y < kHeight)) {
            if (priority >= GetPriorityPixel(x, y)) {
                const size_t colorOffset = (y * kPixelPitch) + (x * 2);
                const size_t priorityOffset = (y * kWidth) + x;
                picture_[colorOffset]     = color;
                picture_[colorOffset + 1] = color;
                priority_[priorityOffset] = priority;
            }
        }
    }

    inline void SetHiDPIPixel(size_t x, size_t y, uint8_t color)
    {
        if ((x < kPixelPitch) && (y < kHeight)) {
            size_t offset = (y * kPixelPitch) + x;
            picture_[offset] = color;
        }
    }

private:
    inline void SetPixel(uint8_t x, uint8_t y) {
        if ((x < kWidth) && (y < kHeight)) {
            if (pictureDraw_) {
                size_t offset = (y * kPixelPitch) + (x * 2);
                picture_[offset]     = pictureColor_;
                picture_[offset + 1] = pictureColor_;
            }
            if (priorityDraw_) {
                size_t offset = (y * kWidth) + x;
                priority_[offset] = priorityColor_;
            }
        }
    }

    inline uint8_t GetPicturePixel(uint8_t x, uint8_t y) const noexcept {
        if ((x < kWidth) && (y < kHeight)) {
            size_t offset = (y * kPixelPitch) + (x * 2);
            return picture_[offset];
        }
        else {
            return 4;
        }
    }

    inline uint8_t GetPriorityPixel(uint8_t x, uint8_t y) const noexcept {
        if ((x < kWidth) && (y < kHeight)) {
            size_t offset = (y * kWidth) + x;
            return priority_[offset];
        }
        else {
            return 4;
        }
    }

    bool CanFill(uint8_t x, uint8_t y);
    void DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

private:
    // the actual visible pixels
    std::array<uint8_t, 64000> picture_;
    // the priority screen
    std::array<uint8_t, 32000> priority_;
    // the picture color
    uint8_t pictureColor_;
    // the priority color
    uint8_t priorityColor_;
    // flags
    uint8_t pictureDraw_     : 1;
    uint8_t priorityDraw_    : 1;
};

} // namespace agi
