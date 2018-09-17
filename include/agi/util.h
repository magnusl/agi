#pragma once

#include <agi/array_view.h>
#include <boost/filesystem.hpp>
#include <vector>
#include <stdint.h>

namespace agi {

void ReadFile(const boost::filesystem::path&, std::vector<uint8_t>& result);

array_view<uint8_t> ParseResource(
    array_view<uint8_t> volume,
    size_t offset);

inline uint16_t U16_BE(const uint8_t* data)
{
    return (static_cast<uint16_t>(data[0]) << 8) | data[1];
}

inline uint16_t U16_LE(const uint8_t* data)
{
    return (static_cast<uint16_t>(data[1]) << 8) | data[0];
}

} // namespace agi