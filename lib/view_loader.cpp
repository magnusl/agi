#include <agi/view_loader.h>
#include <agi/source.h>

namespace agi {

ViewLoader::ViewLoader(
    VolumeLoader& volumes,
    const boost::filesystem::path& directoryFile) :
    volumes_(volumes)
{
    agi::ParseDirectoryFile(directoryFile, entries_);
}

std::shared_ptr<View> ViewLoader::GetView(uint8_t index)
{
    if (views_[index]) {
        return views_[index]; // view already loaded
    }

    if (index >= entries_.size()) {
        throw std::invalid_argument(
            "Invalid view index, no such directory entry.");
    }
    auto& entry = entries_[index];
    auto volume = volumes_.GetVolume(entry.volume);
    if (entry.offset >= volume.size()) {
        throw std::invalid_argument("Invalid volume offset for view.");
    }

    // create the view instance and parse the data
    auto pView = std::make_shared<View>();
    // create soure
    Source source(volume.data(), volume.size(), entry.offset);
    ParseViewResource(source, *pView);
    // store the view
    views_[index] = pView;
    // return the loaded view
    return pView;
}

} // namespace agi
