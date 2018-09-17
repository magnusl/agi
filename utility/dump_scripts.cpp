#include <agi/directory.h>
#include <agi/volume.h>
#include <agi/logic.h>
#include <agi/interpreter.h>
#include <iostream>
#include <map>
#include <sstream>
#include <assert.h>

using VolumeCache = std::map<unsigned, std::shared_ptr<agi::Volume> >;

std::shared_ptr<agi::Volume> FindVolume(
	const std::string& basepath, unsigned volume, VolumeCache& cache)
{
	auto it = cache.find(volume);
	if (it != cache.end()) {
		return it->second;
	}
	else {
		std::stringstream ss;
		ss << basepath << "/VOL." << volume;
		auto result = agi::LoadVolume(ss.str());
		if (result) {
			// found the volume, store it in the cache
			cache[volume] = result;
		}
		return result;
	}
}

int main(int argc, char** argv)
{
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " directory" << std::endl;
		return -1;
	}

	const std::string basepath(argv[1]);
	int scriptIndex = atoi(argv[2]);

	// first load the script directory
	std::vector<agi::DirectoryEntry> directory;
	try {
		const std::string logdir = basepath + "/LOGDIR";
		agi::ParseDirectoryFile(logdir, directory);
	}
	catch(std::exception& e) {
		std::cerr << "Caught exception: " << e.what() << std::endl;
		return -1;
	}

	VolumeCache cache;
	auto& entry = directory.at(scriptIndex);
	// find the volume for the script
	if (auto volume = FindVolume(basepath, entry.volume, cache)) {
		assert(entry.offset < volume->data.size());
		agi::Interpreter interpreter;
		interpreter.ExecuteScriptResource(volume->data, entry.offset);
	}
	else {
		std::cerr << "Failed to find volume for script" << std::endl;
		return -1;
	}

	return 0;
}
