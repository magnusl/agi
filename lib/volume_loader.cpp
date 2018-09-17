#include <agi/volume_loader.h>
#include <agi/util.h>
#include <string>

namespace agi {

VolumeLoader::VolumeLoader(const boost::filesystem::path& path) :
    path_(path)
{
    // empty
}

array_view<uint8_t> VolumeLoader::GetVolume(uint8_t index)
{
    if (index > 15) {
        throw std::invalid_argument("Invalid volume index.");
    }

    auto it = volumes_.find(index);
    if (it != volumes_.end()) {
        // volume already loaded, so return the data
        return array_view<uint8_t>(it->second);
    }
    // volume not loaded
    const std::string filename = "VOL." + std::to_string(index);
    auto volPath = path_ / filename;
    if (!boost::filesystem::exists(volPath)) {
        throw std::invalid_argument("The requested volume file does not exist");
    }
    // load the data
    ReadFile(volPath, volumes_[index]);
    // return the data
    return array_view<uint8_t>(volumes_[index]);
}

} // namespace
