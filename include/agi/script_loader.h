#pragma once

#include <agi/array_view.h>
#include <agi/volume_loader.h>
#include <agi/directory.h>
#include <boost/filesystem.hpp>
#include <array>
#include <memory>

namespace agi {

struct Script
{
    array_view<uint8_t> code;               // the script code
    std::vector<char> stringData;           // decrypted string data
    std::vector<const char*> messages;      // points to null-terminated strings in the string data
};

/**
 * \class   ScriptLoader
 */
class ScriptLoader
{
public:
    ScriptLoader(
        VolumeLoader& volumes,
        const boost::filesystem::path& directoryFile);

    /**
     * \brief   Get a specific script
     */
    std::shared_ptr<Script> GetScript(uint8_t index);

    /**
     * \brief   Loads a script
     */
    std::shared_ptr<Script> LoadScript(uint8_t);

protected:
    VolumeLoader& volumes_;
    std::array<std::shared_ptr<Script>, 256> scripts_;
    std::vector<DirectoryEntry> entries_;
};

} // namespace agi

