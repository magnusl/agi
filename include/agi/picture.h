#pragma once

#include <agi/array_view.h>
#include <agi/framebuffer.h>
#include <agi/source.h>
#include <stdint.h>

namespace agi {

/**
 * \brief   Paints a picture to the framebuffer
 */
void DrawPictureResource(
    array_view<uint8_t> volume,
    size_t offset,
    Framebuffer& framebuffer);

/**
 * \brief   Paints the picture to the framebuffer
 */
void DrawPicture(Source& source, Framebuffer& framebuffer);

} // namespace agi
