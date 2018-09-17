#pragma once
#include <stdint.h>
#include <string>
#include <stdexcept>
#include <cstdio>

namespace agi {

class Source
{
public:
    Source(const uint8_t* buffer, size_t size, size_t offset = 0) :
        buffer_(buffer),
        size_(size),
        offset_(offset)
    {
        // empty
    }

    uint8_t GetU8()
    {
        if (offset_ < size_) {
            return buffer_[offset_++];
        }
        else {
            throw std::runtime_error("Attempt to read outside of buffer.");
        }
    }

    uint16_t GetU16_LE()
    {
        uint8_t low = GetU8();
        uint8_t high = GetU8();

        return (static_cast<uint16_t>(high) << 8) | low;
    }

    uint16_t GetU16_BE()
    {
        uint8_t high = GetU8();
        uint8_t low = GetU8();

        return (static_cast<uint16_t>(high) << 8) | low;
    }

    Source SubSource(size_t length)
    {
        if ((offset_ + length) > size_) {
            throw std::runtime_error("Invalid range for sub-source.");
        }
        return Source(&buffer_[offset_], length);
    }

    void SetOffset(size_t offset)
    {
        if (offset_ < size_) {
            offset_ = offset;
        }
        else {
            throw std::runtime_error("Invalid offset.");
        }
    }

    bool empty() const noexcept
    {
        return offset_ >= size_;
    }

    uint8_t Peek() const
    {
        if (offset_ < size_) {
            return buffer_[offset_];
        }
        else {
            throw std::runtime_error("Attempt to read outside of buffer.");
        }
    }

    void Dump(const char* filename)
    {
        if (FILE* fp = fopen(filename, "wb")) {
            fwrite(buffer_, size_, 1, fp);
            fclose(fp);
        }
    }

    size_t GetOffset() const noexcept { return offset_; }
    size_t GetSize() const noexcept { return size_; }

private:
    const uint8_t* buffer_;
    size_t size_;
    size_t offset_ = 0;
};

} // namespace agi

