#include <agi/directory.h>
#include <agi/util.h>
#include <boost/filesystem/fstream.hpp>
#include <stdexcept>
#include <cstdio>
#include <iostream>

namespace agi {

void ParseDirectoryFile(
    const boost::filesystem::path& directoryFile, std::vector<DirectoryEntry>& result)
{
    std::vector<uint8_t> data;
    ReadFile(directoryFile, data);

    if (data.size() % 3) {
        throw std::runtime_error(
            "The size of a directory file must be a multiple of three.");
    }

    size_t count = data.size() / 3;
    result.resize(count);

    size_t byteOffset = 0;
    for(auto& entry : result) {
        entry.volume = data[byteOffset] >> 4;
        entry.offset =
            (static_cast<uint32_t>(data[byteOffset] & 0x0f) << 16) |
            (static_cast<uint32_t>(data[byteOffset + 1]) << 8) |
            static_cast<uint32_t>(data[byteOffset + 2]);
        byteOffset += 3;
    }
}

} // namespace agi
