#pragma once

#include <agi/util.h>
#include <memory>
#include <iostream>
#include <stdexcept>

namespace agi {

struct Volume
{
	std::vector<uint8_t> data;
};

/**
 * \brief 	Loads a volume
 */
inline std::shared_ptr<Volume> LoadVolume(const std::string& filename)
{
	auto result = std::make_shared<Volume>();
	try {
		ReadFile(filename.c_str(), result->data);
		return result;
	}
	catch(std::invalid_argument&) {
		return nullptr;
	}
}

} // namespace agi
