#include "decoder.h"
#include <fstream>
#include <cstring>
#include <sstream>
#include <iomanip>

namespace wasm {

namespace {
    // WebAssembly binary format constants
    constexpr uint32_t WASM_MAGIC = 0x6D736100;  // "\0asm" in little-endian
    constexpr uint32_t WASM_VERSION = 1;         // MVP version

    // Section IDs as defined in the WebAssembly specification
    constexpr uint8_t SEC_CUSTOM = 0;    // Custom section (name, debug info, etc.)
    constexpr uint8_t SEC_TYPE = 1;      // Function type signatures
    constexpr uint8_t SEC_IMPORT = 2;    // Import declarations
    constexpr uint8_t SEC_FUNCTION = 3;  // Function type indices
    constexpr uint8_t SEC_TABLE = 4;     // Table definitions
    constexpr uint8_t SEC_MEMORY = 5;    // Memory definitions
    constexpr uint8_t SEC_GLOBAL = 6;    // Global variable definitions
    constexpr uint8_t SEC_EXPORT = 7;    // Export declarations
    constexpr uint8_t SEC_START = 8;     // Start function index
    constexpr uint8_t SEC_ELEMENT = 9;   // Element segments (table initialization)
    constexpr uint8_t SEC_CODE = 10;     // Function bodies
    constexpr uint8_t SEC_DATA = 11;     // Data segments (memory initialization)

    // Instruction opcodes
    constexpr uint8_t OP_END = 0x0B;     // End of block/function
}

Module Decoder::parse(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw DecoderError("Failed to open file: " + filename);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer_.resize(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer_.data()), size)) {
        throw DecoderError("Failed to read file: " + filename);
    }

    return parseBytes(buffer_);
}

Module Decoder::parseBytes(const std::vector<uint8_t>& bytes) {
    buffer_ = bytes;
    position_ = 0;

    Module module;

    verifyMagicAndVersion();
    parseModule(module);

    return module;
}

void Decoder::verifyMagicAndVersion() {
    if (buffer_.size() < 8) {
        throw DecoderError("File too small to be a valid WASM module");
    }

    uint32_t magic = readU32();
    if (magic != WASM_MAGIC) {
        throw DecoderError("Invalid magic number");
    }

    uint32_t version = readU32();
    if (version != WASM_VERSION) {
        throw DecoderError("Unsupported version");
    }
}

void Decoder::parseModule(Module& module) {
    module.has_start_function = false;

    while (hasMoreData()) {
        uint8_t section_id = readByte();
        uint32_t section_size = readVarUint32();

        size_t section_start = position_;
        parseSection(module, section_id, section_size);

        // Skip any unparsed bytes in the section
        size_t expected_end = section_start + section_size;
        if (position_ < expected_end) {
            position_ = expected_end;
        }
    }
}

void Decoder::parseSection(Module& module, uint8_t section_id, uint32_t section_size) {
    // Parse the section based on its ID
    // Note: section_size is used by the caller for bounds checking
    (void)section_size;  // Suppress unused parameter warning

    switch (section_id) {
        case SEC_TYPE:
            parseTypeSection(module);
            break;
        case SEC_IMPORT:
            parseImportSection(module);
            break;
        case SEC_FUNCTION:
            parseFunctionSection(module);
            break;
        case SEC_TABLE:
            parseTableSection(module);
            break;
        case SEC_MEMORY:
            parseMemorySection(module);
            break;
        case SEC_GLOBAL:
            parseGlobalSection(module);
            break;
        case SEC_EXPORT:
            parseExportSection(module);
            break;
        case SEC_START:
            parseStartSection(module);
            break;
        case SEC_ELEMENT:
            parseElementSection(module);
            break;
        case SEC_CODE:
            parseCodeSection(module);
            break;
        case SEC_DATA:
            parseDataSection(module);
            break;
        case SEC_CUSTOM:
            // Skip custom sections
            break;
        default:
            throw DecoderError("Unknown section ID: " + std::to_string(section_id));
    }
}

void Decoder::parseTypeSection(Module& module) {
    // Type section: vector of function type signatures
    // Format: count (varuint32) followed by function types
    uint32_t count = readVarUint32();
    module.types.reserve(count);

    for (uint32_t i = 0; i < count; i++) {
        try {
            module.types.push_back(readFuncType());
        } catch (const DecoderError& e) {
            throw DecoderError(formatError("In type section, entry " +
                             std::to_string(i) + ": " + e.what()));
        }
    }
}

void Decoder::parseFunctionSection(Module& module) {
    // Function section: vector of type indices for each function
    // Format: count (varuint32) followed by type indices
    // Note: actual function bodies come from the Code section
    uint32_t count = readVarUint32();
    module.function_types.reserve(count);

    for (uint32_t i = 0; i < count; i++) {
        uint32_t type_index = readVarUint32();
        module.function_types.push_back(type_index);
    }
}

void Decoder::parseTableSection(Module& module) {
    // Table section: vector of table definitions
    // Format: count followed by (element_type, limits) pairs
    // Element type is typically 0x70 (funcref) in MVP
    uint32_t count = readVarUint32();
    module.tables.reserve(count);

    for (uint32_t i = 0; i < count; i++) {
        ValueType elem_type = readValueType();
        Limits limits = readLimits();
        module.tables.emplace_back(elem_type, limits);
    }
}

void Decoder::parseMemorySection(Module& module) {
    // Memory section: vector of memory definitions
    // Format: count followed by limits (min pages, optional max pages)
    // Each page is 64KB (65536 bytes)
    uint32_t count = readVarUint32();
    module.memories.reserve(count);

    for (uint32_t i = 0; i < count; i++) {
        Limits limits = readLimits();
        module.memories.emplace_back(limits);
    }
}

void Decoder::parseGlobalSection(Module& module) {
    // Global section: vector of global variable definitions
    // Format: count followed by (type, mutability, init_expr) triples
    // Init expression is a constant expression that initializes the global
    uint32_t count = readVarUint32();
    module.globals.reserve(count);

    for (uint32_t i = 0; i < count; i++) {
        Global global;
        global.type = readValueType();
        global.is_mutable = readByte() != 0;

        // Read initialization expression
        // This is a constant expression (e.g., i32.const 0) followed by END
        global.init_expr = readInitExpression();

        module.globals.push_back(std::move(global));
    }
}

void Decoder::parseExportSection(Module& module) {
    // Export section: vector of exports
    // Format: count followed by (name, kind, index) triples
    // Exports make internal items accessible from outside the module
    uint32_t count = readVarUint32();
    module.exports.reserve(count);

    for (uint32_t i = 0; i < count; i++) {
        std::string name = readName();
        ExternalKind kind = static_cast<ExternalKind>(readByte());
        uint32_t index = readVarUint32();

        module.exports.emplace_back(std::move(name), kind, index);
    }
}

void Decoder::parseStartSection(Module& module) {
    // Start section: optional function to call automatically on instantiation
    // Format: single function index
    module.start_function_index = readVarUint32();
    module.has_start_function = true;
}

void Decoder::parseElementSection(Module& module) {
    // Element section: vector of element segments for table initialization
    // Format: count followed by (table_index, offset_expr, func_indices) triples
    // Used to initialize tables with function references
    uint32_t count = readVarUint32();
    module.element_segments.reserve(count);

    for (uint32_t i = 0; i < count; i++) {
        ElementSegment segment;
        segment.table_index = readVarUint32();

        // Read offset expression that determines where in the table to start
        segment.offset_expr = readInitExpression();

        // Read vector of function indices to place in the table
        uint32_t elem_count = readVarUint32();
        segment.func_indices.reserve(elem_count);
        for (uint32_t j = 0; j < elem_count; j++) {
            segment.func_indices.push_back(readVarUint32());
        }

        module.element_segments.push_back(std::move(segment));
    }
}

void Decoder::parseCodeSection(Module& module) {
    // Code section: vector of function bodies
    // Format: count followed by function bodies
    // Must have same count as Function section
    // Each body: size, locals, instructions
    uint32_t count = readVarUint32();
    module.functions.reserve(count);

    // Validate that code count matches function count
    if (count != module.function_types.size()) {
        throw DecoderError(formatError("Code section count (" + std::to_string(count) +
                         ") does not match function section count (" +
                         std::to_string(module.function_types.size()) + ")"));
    }

    for (uint32_t i = 0; i < count; i++) {
        uint32_t body_size = readVarUint32();
        size_t body_start = position_;

        Function func;
        func.type_index = module.function_types[i];

        // Parse locals: compressed format with (count, type) pairs
        // Example: [(2, i32), (1, i64)] means 2 i32 locals and 1 i64 local
        uint32_t local_decl_count = readVarUint32();
        for (uint32_t j = 0; j < local_decl_count; j++) {
            uint32_t local_count = readVarUint32();
            ValueType type = readValueType();
            for (uint32_t k = 0; k < local_count; k++) {
                func.locals.push_back(type);
            }
        }

        // Read function body bytecode (instructions)
        // Body ends with 0x0B (END instruction)
        size_t body_bytes = body_size - (position_ - body_start);
        func.body = readBytes(body_bytes);

        module.functions.push_back(std::move(func));
    }
}

void Decoder::parseDataSection(Module& module) {
    // Data section: vector of data segments for memory initialization
    // Format: count followed by (memory_index, offset_expr, data) triples
    // Used to initialize memory with constant data
    uint32_t count = readVarUint32();
    module.data_segments.reserve(count);

    for (uint32_t i = 0; i < count; i++) {
        DataSegment segment;
        segment.memory_index = readVarUint32();

        // Read offset expression that determines where in memory to place data
        segment.offset_expr = readInitExpression();

        // Read the actual data bytes
        uint32_t data_size = readVarUint32();
        segment.data = readBytes(data_size);

        module.data_segments.push_back(std::move(segment));
    }
}

void Decoder::parseImportSection(Module& module) {
    // Import section: vector of imports
    // Format: count followed by (module_name, field_name, kind, type_info) tuples
    // Imports bring in external functionality
    uint32_t count = readVarUint32();
    module.imports.reserve(count);

    for (uint32_t i = 0; i < count; i++) {
        Import import;
        import.module_name = readName();
        import.field_name = readName();
        import.kind = static_cast<ExternalKind>(readByte());

        // Read type-specific information based on import kind
        switch (import.kind) {
            case ExternalKind::FUNCTION:
                // Function import: type index into type section
                import.type_index = readVarUint32();
                break;
            case ExternalKind::TABLE:
                // Table import: element type and limits
                import.table.element_type = readValueType();
                import.table.limits = readLimits();
                break;
            case ExternalKind::MEMORY:
                // Memory import: limits
                import.memory.limits = readLimits();
                break;
            case ExternalKind::GLOBAL:
                // Global import: type and mutability
                import.global.type = readValueType();
                import.global.is_mutable = readByte() != 0;
                break;
        }

        module.imports.push_back(std::move(import));
    }
}

// Helper functions

std::vector<uint8_t> Decoder::readInitExpression() {
    // Read an initialization expression (constant expression)
    // Format: instruction(s) followed by END (0x0B)
    // Common examples: i32.const 0, global.get 0, etc.
    std::vector<uint8_t> expr;

    while (true) {
        uint8_t byte = readByte();
        expr.push_back(byte);

        if (byte == OP_END) {
            break;
        }

        // Safety check to prevent infinite loops on malformed input
        if (expr.size() > 1024) {
            throw DecoderError(formatError("Init expression too large (> 1024 bytes)"));
        }
    }

    return expr;
}

std::string Decoder::formatError(const std::string& message) const {
    std::ostringstream oss;
    oss << "At byte 0x" << std::hex << std::setfill('0') << std::setw(4) << position_
        << std::dec << " (" << position_ << "): " << message;
    return oss.str();
}

// Binary reading utilities

uint8_t Decoder::readByte() {
    ensureBytes(1);
    return buffer_[position_++];
}

uint32_t Decoder::readU32() {
    // Read unsigned 32-bit integer in little-endian format
    ensureBytes(4);
    uint32_t value;
    std::memcpy(&value, &buffer_[position_], 4);
    position_ += 4;
    return value;
}

int32_t Decoder::readI32() {
    // Read signed 32-bit integer in little-endian format
    return static_cast<int32_t>(readU32());
}

int64_t Decoder::readI64() {
    // Read signed 64-bit integer in little-endian format
    ensureBytes(8);
    int64_t value;
    std::memcpy(&value, &buffer_[position_], 8);
    position_ += 8;
    return value;
}

float Decoder::readF32() {
    // Read IEEE 754 single-precision float in little-endian format
    ensureBytes(4);
    float value;
    std::memcpy(&value, &buffer_[position_], 4);
    position_ += 4;
    return value;
}

double Decoder::readF64() {
    // Read IEEE 754 double-precision float in little-endian format
    ensureBytes(8);
    double value;
    std::memcpy(&value, &buffer_[position_], 8);
    position_ += 8;
    return value;
}

std::vector<uint8_t> Decoder::readBytes(size_t count) {
    // Read a sequence of raw bytes
    ensureBytes(count);
    std::vector<uint8_t> result(buffer_.begin() + position_,
                                 buffer_.begin() + position_ + count);
    position_ += count;
    return result;
}

std::string Decoder::readName() {
    // Read a UTF-8 string with LEB128 length prefix
    // Format: length (varuint32) followed by UTF-8 bytes
    uint32_t length = readVarUint32();
    ensureBytes(length);

    std::string name(reinterpret_cast<const char*>(&buffer_[position_]), length);
    position_ += length;
    return name;
}

// LEB128 decoding
// LEB128 (Little Endian Base 128) is a variable-length encoding for integers
// Each byte contains 7 bits of data and 1 continuation bit (MSB)
// Continuation bit = 1 means more bytes follow, 0 means last byte

uint32_t Decoder::readVarUint32() {
    // Decode unsigned 32-bit LEB128 integer
    // Maximum 5 bytes for 32-bit value (5 * 7 = 35 bits, covers 32)
    uint32_t result = 0;
    int shift = 0;

    while (true) {
        uint8_t byte = readByte();
        result |= (byte & 0x7F) << shift;

        // Check if this is the last byte (continuation bit = 0)
        if ((byte & 0x80) == 0) {
            break;
        }

        shift += 7;
        if (shift >= 35) {
            throw DecoderError(formatError("Invalid LEB128 unsigned encoding (too many bytes)"));
        }
    }

    return result;
}

uint64_t Decoder::readVarUint64() {
    // Decode unsigned 64-bit LEB128 integer
    // Maximum 10 bytes for 64-bit value (10 * 7 = 70 bits, covers 64)
    uint64_t result = 0;
    int shift = 0;

    while (true) {
        uint8_t byte = readByte();
        result |= static_cast<uint64_t>(byte & 0x7F) << shift;

        // Check if this is the last byte (continuation bit = 0)
        if ((byte & 0x80) == 0) {
            break;
        }

        shift += 7;
        if (shift >= 70) {
            throw DecoderError(formatError("Invalid LEB128 unsigned encoding (too many bytes)"));
        }
    }

    return result;
}

int32_t Decoder::readVarInt32() {
    // Decode signed 32-bit LEB128 integer with sign extension
    int32_t result = 0;
    int shift = 0;
    uint8_t byte;

    do {
        byte = readByte();
        result |= (byte & 0x7F) << shift;
        shift += 7;
    } while ((byte & 0x80) != 0);

    // Sign extend if the sign bit (bit 6 of last byte) is set
    if (shift < 32 && (byte & 0x40)) {
        result |= -(1 << shift);
    }

    return result;
}

int64_t Decoder::readVarInt64() {
    // Decode signed 64-bit LEB128 integer with sign extension
    int64_t result = 0;
    int shift = 0;
    uint8_t byte;

    do {
        byte = readByte();
        result |= static_cast<int64_t>(byte & 0x7F) << shift;
        shift += 7;
    } while ((byte & 0x80) != 0);

    // Sign extend if the sign bit (bit 6 of last byte) is set
    if (shift < 64 && (byte & 0x40)) {
        result |= -(1LL << shift);
    }

    return result;
}

ValueType Decoder::readValueType() {
    // Read a value type byte
    // 0x7F = i32, 0x7E = i64, 0x7D = f32, 0x7C = f64
    // 0x70 = funcref (for tables), 0x40 = void/empty
    uint8_t byte = readByte();
    return static_cast<ValueType>(byte);
}

FuncType Decoder::readFuncType() {
    // Read a function type signature
    // Format: 0x60 (form) followed by parameter types and result types
    // Example: 0x60 0x02 0x7F 0x7F 0x01 0x7F = func(i32, i32) -> i32
    uint8_t form = readByte();
    if (form != 0x60) {
        throw DecoderError(formatError("Invalid function type form: expected 0x60, got 0x" +
                         std::to_string(form)));
    }

    FuncType func_type;

    // Read parameter types
    uint32_t param_count = readVarUint32();
    func_type.params.reserve(param_count);
    for (uint32_t i = 0; i < param_count; i++) {
        func_type.params.push_back(readValueType());
    }

    // Read result types (MVP allows 0 or 1 result)
    uint32_t result_count = readVarUint32();
    func_type.results.reserve(result_count);
    for (uint32_t i = 0; i < result_count; i++) {
        func_type.results.push_back(readValueType());
    }

    return func_type;
}

Limits Decoder::readLimits() {
    // Read memory or table limits
    // Format: flags byte followed by min (and optionally max)
    // flags & 0x01: 0 = no max, 1 = has max
    uint8_t flags = readByte();
    uint32_t min = readVarUint32();

    if (flags & 0x01) {
        uint32_t max = readVarUint32();
        return Limits(min, max);
    }

    return Limits(min);
}

bool Decoder::hasMoreData() const {
    return position_ < buffer_.size();
}

void Decoder::ensureBytes(size_t count) const {
    if (position_ + count > buffer_.size()) {
        throw DecoderError("Unexpected end of file");
    }
}

} // namespace wasm
