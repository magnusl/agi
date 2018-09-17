#pragma once

#include <agi/volume_loader.h>
#include <agi/directory.h>
#include <agi/view.h>
#include <vector>
#include <array>
#include <stdint.h>

namespace agi {

/**
 * \class   ViewLoader
 */
class ViewLoader
{
public:
    ViewLoader(
        VolumeLoader& volumes,
        const boost::filesystem::path& directoryFile);

    /**
     * \brief   Get a specific script
     */
    std::shared_ptr<View> GetView(uint8_t index);

private:
    VolumeLoader& volumes_;
    std::array<std::shared_ptr<View>, 256> views_;
    std::vector<DirectoryEntry> entries_;
};

} // namespace agi

