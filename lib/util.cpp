#include <agi/util.h>
#include <agi/commands.h>
#include <boost/filesystem/fstream.hpp>
#include <iostream>
#include <assert.h>

using namespace boost;

namespace agi {

array_view<uint8_t> ParseResource(
    array_view<uint8_t> volume,
    size_t offset)
{
    // make sure that the 5 bytes header fit inside the volume
    if ((offset > volume.size()) || ((volume.size() - offset) < 5)) {
        throw std::runtime_error(
            "Invalid resource, header does not fit inside volume.");
    }

    if (U16_BE(&volume[offset]) != 0x1234) {
        throw std::runtime_error(
            "Invalid resource header, expected magic number");
    }

    // the length of the resource
    uint16_t length = U16_LE(&volume[offset + 3]);
    // make sure that the length is valid
    if ((volume.size() - offset) < (length + 5)) {
        throw std::runtime_error(
            "Resource does not fit inside volume.");
    }

    // return an view of the resource data
    return array_view<uint8_t>(&volume[offset + 5], length);
}

void ReadFile(const boost::filesystem::path& filename, std::vector<uint8_t>& result)
{
    try {
        if (!filesystem::exists(filename) ||
            !filesystem::is_regular_file(filename))
        {
            throw std::invalid_argument("Could not open file");
        }
        // determine file size, and check if we can read the entire file or not
        auto size = filesystem::file_size(filename);
        if (0 == size) {
            throw std::runtime_error("File is empty, can't read any data");
        }

        // open file for reading
        boost::filesystem::ifstream input(filename);
        if (!input) {
            throw std::runtime_error("Failed to open file for reading.");

        }
        // disable skipping of whitespace
        input >> std::noskipws;

        // copy the file
        result.reserve(size);
        std::istream_iterator<uint8_t> it(input);
        std::copy(it, std::istream_iterator<uint8_t>(),std::back_inserter(result));
    }
    catch(filesystem::filesystem_error&) {
        throw std::runtime_error("Failed to read file.");
    }
    catch(std::bad_alloc&) {
        throw std::runtime_error("Failed to allocate memory for file buffer.");
    }
}

namespace {

static const CommandType CmdTypes[] = {
    CommandType::kProgramControl,
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    // NewRoom
    CommandType::kProgramControl,
    CommandType::kProgramControl,
    // LoadLogics
    CommandType::kResourceManagement,
    CommandType::kResourceManagement,
    // Call
    CommandType::kProgramControl,
    CommandType::kProgramControl,
    // LoadPic
    CommandType::kResourceManagement,
    // DrawPic
    CommandType::kPictureManagement,
    CommandType::kPictureManagement,
    // DiscardPic
    CommandType::kResourceManagement,
    CommandType::kPictureManagement,
    // ShowPriScreen
    CommandType::kOther,
    CommandType::kResourceManagement,
    CommandType::kResourceManagement,
    CommandType::kResourceManagement,
    // AnimateObj
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    // Reposition
    CommandType::kObjectMotion,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    // StopUpdate
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    // IgnoreObjects
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    // StopCycling
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    CommandType::kObjectDescription,
    // StopMotion
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    // Get
    CommandType::kInventoryItem,
    CommandType::kInventoryItem,
    CommandType::kInventoryItem,
    CommandType::kInventoryItem,
    CommandType::kInventoryItem,
    CommandType::kInventoryItem,
    // LoadSound
    CommandType::kSoundManagement,
    CommandType::kSoundManagement,
    CommandType::kSoundManagement,
    // Print
    CommandType::kTextManagement,
    CommandType::kTextManagement,
    CommandType::kTextManagement,
    CommandType::kTextManagement,
    CommandType::kTextManagement,
    CommandType::kTextManagement,
    CommandType::kTextManagement,
    CommandType::kTextManagement,
    CommandType::kTextManagement,
    // ShakeScreen
    CommandType::kOther,
    CommandType::kOther,
    // StatusLineOn
    CommandType::kTextManagement,
    CommandType::kTextManagement,
    // SetString
    CommandType::kStringManagement,
    CommandType::kStringManagement,
    CommandType::kStringManagement,
    CommandType::kStringManagement,
    CommandType::kStringManagement,
    // PreventInput
    CommandType::kTextManagement,
    CommandType::kTextManagement,
    // SetKey
    CommandType::kInitialization,
    // AddToPic
    CommandType::kPictureManagement,
    CommandType::kPictureManagement,
    // Status
    CommandType::kInventoryItem,
    // SaveGame
    CommandType::kOther,
    CommandType::kOther,
    CommandType::kOther,    
    CommandType::kOther,
    // ShowObj
    CommandType::kOther,
    CommandType::kArithmetic,
    // ProgramControl
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kOther,
    
    CommandType::kOther,
    CommandType::kOther,
    CommandType::kOther,
    CommandType::kOther,
    CommandType::kOther,
    CommandType::kOther,
    CommandType::kOther,
    // version
    CommandType::kTextManagement,
    CommandType::kInitialization,
    CommandType::kInitialization,
    CommandType::kInitialization,
    // SetScanStart
    CommandType::kProgramControl,
    CommandType::kProgramControl,
    CommandType::kObjectMotion,
    CommandType::kObjectMotion,
    CommandType::kInitialization,
    CommandType::kInitialization,
    // PrintAt
    CommandType::kTextManagement,
    CommandType::kTextManagement,    
    CommandType::kResourceManagement,
    CommandType::kTextManagement,    
    // SetUpperLeft
    CommandType::kOther,    
    CommandType::kMenuManagement,
    CommandType::kMenuManagement,
    CommandType::kMenuManagement,
    CommandType::kMenuManagement,
    CommandType::kMenuManagement,
    CommandType::kMenuManagement,
    CommandType::kOther,
    // OpenDialogue
    CommandType::kOther,
    CommandType::kOther,
    // MulN
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    CommandType::kArithmetic,
    // CloseWindow
    CommandType::kOther,
};

} // namespace

CommandType GetCommandType(uint8_t command)
{
    assert(command < sizeof(CmdTypes) / sizeof(CmdTypes[0]));
    return CmdTypes[command];
}

} // namespace agi
