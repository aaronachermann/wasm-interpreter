#include "memory.h"
#include <cstring>
#include <algorithm>

namespace wasm {

Memory::Memory(const Limits& limits) : limits_(limits), current_pages_(limits.min) {
    if (limits.min > MAX_PAGES) {
        throw MemoryError("Initial memory size exceeds maximum");
    }

    if (limits.has_max && limits.max > MAX_PAGES) {
        throw MemoryError("Maximum memory size exceeds limit");
    }

    if (limits.has_max && limits.min > limits.max) {
        throw MemoryError("Initial size exceeds maximum size");
    }

    data_.resize(current_pages_ * PAGE_SIZE, 0);
}

// Load operations

int32_t Memory::loadI32(uint32_t address) const {
    return load<int32_t>(address);
}

int64_t Memory::loadI64(uint32_t address) const {
    return load<int64_t>(address);
}

float Memory::loadF32(uint32_t address) const {
    return load<float>(address);
}

double Memory::loadF64(uint32_t address) const {
    return load<double>(address);
}

uint8_t Memory::loadU8(uint32_t address) const {
    return load<uint8_t>(address);
}

uint16_t Memory::loadU16(uint32_t address) const {
    return load<uint16_t>(address);
}

uint32_t Memory::loadU32(uint32_t address) const {
    return load<uint32_t>(address);
}

uint64_t Memory::loadU64(uint32_t address) const {
    return load<uint64_t>(address);
}

int8_t Memory::loadI8(uint32_t address) const {
    return load<int8_t>(address);
}

int16_t Memory::loadI16(uint32_t address) const {
    return load<int16_t>(address);
}

// Store operations

void Memory::storeI32(uint32_t address, int32_t value) {
    store(address, value);
}

void Memory::storeI64(uint32_t address, int64_t value) {
    store(address, value);
}

void Memory::storeF32(uint32_t address, float value) {
    store(address, value);
}

void Memory::storeF64(uint32_t address, double value) {
    store(address, value);
}

void Memory::storeU8(uint32_t address, uint8_t value) {
    store(address, value);
}

void Memory::storeU16(uint32_t address, uint16_t value) {
    store(address, value);
}

void Memory::storeU32(uint32_t address, uint32_t value) {
    store(address, value);
}

void Memory::storeU64(uint32_t address, uint64_t value) {
    store(address, value);
}

// Memory operations

int32_t Memory::grow(uint32_t delta) {
    if (delta == 0) {
        return static_cast<int32_t>(current_pages_);
    }

    uint32_t new_pages = current_pages_ + delta;

    // Check overflow
    if (new_pages < current_pages_) {
        return -1;
    }

    // Check against maximum
    if (new_pages > MAX_PAGES) {
        return -1;
    }

    if (limits_.has_max && new_pages > limits_.max) {
        return -1;
    }

    int32_t old_pages = static_cast<int32_t>(current_pages_);
    current_pages_ = new_pages;
    data_.resize(current_pages_ * PAGE_SIZE, 0);

    return old_pages;
}

void Memory::initialize(uint32_t offset, const std::vector<uint8_t>& data) {
    if (offset + data.size() > data_.size()) {
        throw MemoryError("Data segment out of bounds");
    }

    std::memcpy(data_.data() + offset, data.data(), data.size());
}

void Memory::clear() {
    std::fill(data_.begin(), data_.end(), 0);
}

// Private methods

void Memory::checkAddress(uint32_t address, size_t size) const {
    if (address + size > data_.size() || address + size < address) {
        throw MemoryError("Memory access out of bounds");
    }
}

template<typename T>
T Memory::load(uint32_t address) const {
    checkAddress(address, sizeof(T));
    T value;
    std::memcpy(&value, &data_[address], sizeof(T));
    return value;
}

template<typename T>
void Memory::store(uint32_t address, T value) {
    checkAddress(address, sizeof(T));
    std::memcpy(&data_[address], &value, sizeof(T));
}

// Explicit template instantiations
template int8_t Memory::load<int8_t>(uint32_t) const;
template int16_t Memory::load<int16_t>(uint32_t) const;
template int32_t Memory::load<int32_t>(uint32_t) const;
template int64_t Memory::load<int64_t>(uint32_t) const;
template uint8_t Memory::load<uint8_t>(uint32_t) const;
template uint16_t Memory::load<uint16_t>(uint32_t) const;
template uint32_t Memory::load<uint32_t>(uint32_t) const;
template uint64_t Memory::load<uint64_t>(uint32_t) const;
template float Memory::load<float>(uint32_t) const;
template double Memory::load<double>(uint32_t) const;

template void Memory::store<int8_t>(uint32_t, int8_t);
template void Memory::store<int16_t>(uint32_t, int16_t);
template void Memory::store<int32_t>(uint32_t, int32_t);
template void Memory::store<int64_t>(uint32_t, int64_t);
template void Memory::store<uint8_t>(uint32_t, uint8_t);
template void Memory::store<uint16_t>(uint32_t, uint16_t);
template void Memory::store<uint32_t>(uint32_t, uint32_t);
template void Memory::store<uint64_t>(uint32_t, uint64_t);
template void Memory::store<float>(uint32_t, float);
template void Memory::store<double>(uint32_t, double);

} // namespace wasm
