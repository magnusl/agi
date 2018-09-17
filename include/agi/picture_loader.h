#pragma once

#include <agi/framebuffer.h>
#include <agi/volume_loader.h>
#include <agi/directory.h>
#include <boost/filesystem.hpp>
#include <vector>

namespace agi {

/**
 * \class   PictureLoader
 */
class PictureLoader
{
public:
    PictureLoader(
        VolumeLoader& volumes,
        const boost::filesystem::path& directoryFile);

    void DrawPicture(uint8_t picture, Framebuffer&);
    void OverlayPicture(uint8_t picture, Framebuffer&);

private:
    VolumeLoader& volumes_;
    std::vector<DirectoryEntry> entries_;
};

} // namespace agi
