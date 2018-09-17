#include <agi/script_loader.h>
#include <agi/util.h>

#include <stdexcept>
#include <sstream>

namespace agi {

ScriptLoader::ScriptLoader(
    VolumeLoader& volumes,
    const boost::filesystem::path& directoryFile) :
    volumes_(volumes)
{
    agi::ParseDirectoryFile(directoryFile, entries_);
}

std::shared_ptr<Script> ScriptLoader::GetScript(uint8_t index)
{
    if (auto script = scripts_[index]) {
        // script already loaded, so just return it
        return script;
    }
    else {
        // script is not loaded, so load it and return it
        return LoadScript(index);
    }
}

namespace {
   
void Decrypt(const uint8_t* startp, size_t count, std::vector<char>& result)
{
    const char key[] = "Avis Durgan";
    auto keyLength = strlen(key);

    result.reserve(count);
    for(size_t i = 0; i < count; ++i) {
        result.push_back(startp[i] ^ key[i % keyLength]);
    }
}

std::shared_ptr<Script> ParseScriptResource(
    array_view<uint8_t> volume,
    size_t offset,
    size_t scriptIndex)
{
    auto data = ParseResource(volume, offset);

    // make sure that the text offset fits
    if (data.size() < 2) {
        throw std::runtime_error(
            "Text offset does not fit inside the script resource.");
    }
    // mstart
    const auto messageStart = U16_LE(&data[0]) + 2; // actual offset to messages
    // mc
    const auto messageCount = data[messageStart];   // the number of messages
    const auto messageEnd   = U16_LE(&data[messageStart + 1]) + messageStart + 1;
    const auto messageData  = messageStart + 3 + (messageCount * 2);


    std::stringstream ss;
    ss << "/tmp/script" << ((unsigned) scriptIndex) << ".txt";
    if (FILE* fp=fopen(ss.str().c_str(), "wb")) {
        fwrite(data.data(), data.size(), 1, fp);
        fclose(fp);
    }

    // create script instance
    auto result = std::make_shared<Script>();
    // the actual script
    result->code = array_view<uint8_t>(&data[2], messageStart - 2);
    // decrypt extract the message data
    Decrypt(&data[messageData], messageEnd - messageData, result->stringData);
    // extract the message offsets
    for(size_t i = 0; i < messageCount; ++i) {
        // read the encoded offset
        size_t offsetValue = U16_LE(&data[messageStart + 3 + (i * 2)]);
        size_t stringPos = messageStart + offsetValue + 1;
        if (stringPos < messageData) {
            // Not a valid string
            result->messages.push_back(nullptr);
        }
        else {
            auto bufferIndex = stringPos - messageData;
            result->messages.push_back(&result->stringData[bufferIndex]);
        }
    }
    return result;
}

} // namespace

std::shared_ptr<Script> ScriptLoader::LoadScript(uint8_t index)
{
    if (auto script = scripts_[index]) {
        // script already loaded, so just return it
        return script;
    }

    if (index >= entries_.size()) {
        throw std::invalid_argument(
            "Invalid script index, no such directory entry.");
    }
    auto& entry = entries_[index];

    // get the volume that contains the script
    auto volume = volumes_.GetVolume(entry.volume);
    // make sure that the offset is valid
    if (entry.offset >= volume.size()) {
        throw std::invalid_argument("Invalid volume offset.");
    }

    scripts_[index] = ParseScriptResource(volume, entry.offset, index);

    return scripts_[index];
}

} // namespace agi
