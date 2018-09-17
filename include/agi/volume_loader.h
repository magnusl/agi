#pragma once

#include <agi/array_view.h>
#include <boost/filesystem.hpp>
#include <stdint.h>
#include <map>
#include <vector>

namespace agi {

/**
 * \class   VolumeLoader
 */
class VolumeLoader
{
public:
    VolumeLoader(const boost::filesystem::path&);

    /**
     * \brief   Get the data of a specific volume. Once the volume has been
     *          loaded, it will be kept in memory indefinitly.
     */
    array_view<uint8_t> GetVolume(uint8_t index);
    
private:
    const boost::filesystem::path path_;
    std::map<uint8_t, std::vector<uint8_t> > volumes_;
};

} // namespace agi

