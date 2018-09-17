#pragma once

#include <boost/filesystem.hpp>
#include <stdint.h>
#include <vector>
#include <string>

#define MAX_DIRECTORY_ENTRIES (256)
#define DIRECTORY_ENTRY_SIZE (3)

namespace agi {

/**
 * \struct DirectoryEntry
 */
struct DirectoryEntry
{
    uint32_t volume : 4;
    uint32_t offset : 28;
};

/**
 * \brief   Parses a directory file
 */
void ParseDirectoryFile(
	const boost::filesystem::path& directoryFile, std::vector<DirectoryEntry>& result);

} // namespace agi
