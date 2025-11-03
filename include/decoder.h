#ifndef WASM_DECODER_H
#define WASM_DECODER_H

#include "module.h"
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>

namespace wasm {

/**
 * Exception thrown when decoding fails.
 */
class DecoderError : public std::runtime_error {
public:
    explicit DecoderError(const std::string& message)
        : std::runtime_error("Decoder error: " + message) {}
};

/**
 * Decoder for WebAssembly binary format.
 * Parses .wasm files and constructs Module objects.
 */
class Decoder {
public:
    Decoder() = default;
    ~Decoder() = default;

    /**
     * Parse a WebAssembly binary file and return a Module.
     * @param filename Path to the .wasm file
     * @return Parsed module
     * @throws DecoderError if parsing fails
     */
    Module parse(const std::string& filename);

    /**
     * Parse from raw bytes.
     * @param bytes Binary data
     * @return Parsed module
     * @throws DecoderError if parsing fails
     */
    Module parseBytes(const std::vector<uint8_t>& bytes);

private:
    std::vector<uint8_t> buffer_;
    size_t position_;

    // Binary reading utilities
    uint8_t readByte();
    uint32_t readU32();
    int32_t readI32();
    int64_t readI64();
    float readF32();
    double readF64();
    std::vector<uint8_t> readBytes(size_t count);
    std::string readName();

    // LEB128 decoding
    uint32_t readVarUint32();
    uint64_t readVarUint64();
    int32_t readVarInt32();
    int64_t readVarInt64();

    // Type parsing
    ValueType readValueType();
    FuncType readFuncType();
    Limits readLimits();

    // Section parsing
    void parseModule(Module& module);
    void verifyMagicAndVersion();
    void parseSection(Module& module, uint8_t section_id, uint32_t section_size);

    void parseTypeSection(Module& module);
    void parseFunctionSection(Module& module);
    void parseTableSection(Module& module);
    void parseMemorySection(Module& module);
    void parseGlobalSection(Module& module);
    void parseExportSection(Module& module);
    void parseStartSection(Module& module);
    void parseElementSection(Module& module);
    void parseCodeSection(Module& module);
    void parseDataSection(Module& module);
    void parseImportSection(Module& module);

    // Helper for reading init expressions
    std::vector<uint8_t> readInitExpression();

    // Helper for reading vectors
    template<typename T, typename ParseFunc>
    std::vector<T> readVector(ParseFunc parse_func);

    // Validation
    bool hasMoreData() const;
    void ensureBytes(size_t count) const;

    // Error reporting with context
    std::string formatError(const std::string& message) const;
};

} // namespace wasm

#endif // WASM_DECODER_H
