#include <agi/picture_loader.h>
#include <agi/picture.h>

namespace agi {

PictureLoader::PictureLoader(
    VolumeLoader& volumes,
    const boost::filesystem::path& directoryFile) :
    volumes_(volumes)
{
    agi::ParseDirectoryFile(directoryFile, entries_);
}

void PictureLoader::DrawPicture(uint8_t picture, Framebuffer& framebuffer)
{
    framebuffer.Clear();
    OverlayPicture(picture, framebuffer);
}

void PictureLoader::OverlayPicture(uint8_t picture, Framebuffer& framebuffer)
{
    if (picture >= entries_.size()) {
        throw std::invalid_argument(
            "Invalid script index, no such directory entry.");
    }
    auto& entry = entries_[picture];

    auto volume = volumes_.GetVolume(entry.volume);
    // make sure that the offset is valid
    if (entry.offset >= volume.size()) {
        throw std::invalid_argument("Invalid volume offset.");
    }

    DrawPictureResource(volume, entry.offset, framebuffer);
}

} // namespace agi
