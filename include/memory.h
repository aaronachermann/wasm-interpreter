#ifndef WASM_MEMORY_H
#define WASM_MEMORY_H

#include "types.h"
#include <vector>
#include <cstdint>
#include <stdexcept>

namespace wasm {

/**
 * Exception thrown on memory access errors.
 */
class MemoryError : public std::runtime_error {
public:
    explicit MemoryError(const std::string& message)
        : std::runtime_error("Memory error: " + message) {}
};

/**
 * Linear memory implementation for WebAssembly.
 * Memory is organized in pages of 64KB each.
 */
class Memory {
public:
    static constexpr uint32_t PAGE_SIZE = 65536;  // 64KB
    static constexpr uint32_t MAX_PAGES = 65536;  // 4GB maximum

    Memory() = default;
    explicit Memory(const Limits& limits);
    ~Memory() = default;

    // Load operations (read from memory)
    int32_t loadI32(uint32_t address) const;
    int64_t loadI64(uint32_t address) const;
    float loadF32(uint32_t address) const;
    double loadF64(uint32_t address) const;

    uint8_t loadU8(uint32_t address) const;
    uint16_t loadU16(uint32_t address) const;
    uint32_t loadU32(uint32_t address) const;
    uint64_t loadU64(uint32_t address) const;

    int8_t loadI8(uint32_t address) const;
    int16_t loadI16(uint32_t address) const;

    // Store operations (write to memory)
    void storeI32(uint32_t address, int32_t value);
    void storeI64(uint32_t address, int64_t value);
    void storeF32(uint32_t address, float value);
    void storeF64(uint32_t address, double value);

    void storeU8(uint32_t address, uint8_t value);
    void storeU16(uint32_t address, uint16_t value);
    void storeU32(uint32_t address, uint32_t value);
    void storeU64(uint32_t address, uint64_t value);

    // Memory operations
    /**
     * Grow memory by delta pages.
     * @param delta Number of pages to add
     * @return Previous size in pages, or -1 if growth failed
     */
    int32_t grow(uint32_t delta);

    /**
     * Get current memory size in pages.
     */
    uint32_t size() const { return current_pages_; }

    /**
     * Get current memory size in bytes.
     */
    uint32_t sizeInBytes() const { return current_pages_ * PAGE_SIZE; }

    /**
     * Initialize memory region with data.
     * Used during module instantiation for data segments.
     */
    void initialize(uint32_t offset, const std::vector<uint8_t>& data);

    /**
     * Get raw pointer to memory data (for debugging).
     */
    const uint8_t* data() const { return data_.data(); }

    /**
     * Clear all memory.
     */
    void clear();

private:
    std::vector<uint8_t> data_;
    Limits limits_;
    uint32_t current_pages_;

    // Bounds checking
    void checkAddress(uint32_t address, size_t size) const;

    // Generic load/store with bounds checking
    template<typename T>
    T load(uint32_t address) const;

    template<typename T>
    void store(uint32_t address, T value);
};

} // namespace wasm

#endif // WASM_MEMORY_H
