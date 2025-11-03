#ifndef WASM_MODULE_H
#define WASM_MODULE_H

#include "types.h"
#include <vector>
#include <string>
#include <cstdint>
#include <memory>

namespace wasm {

/**
 * Represents a WebAssembly function with its locals and bytecode.
 */
struct Function {
    uint32_t type_index;                    // Index into type section
    std::vector<ValueType> locals;          // Local variables (excluding parameters)
    std::vector<uint8_t> body;              // Function bytecode

    Function() : type_index(0) {}
};

/**
 * Represents linear memory type descriptor with size limits.
 */
struct MemoryType {
    Limits limits;  // Min/max pages (1 page = 64KB)

    MemoryType() = default;
    explicit MemoryType(Limits l) : limits(std::move(l)) {}
};

/**
 * Represents a global variable.
 */
struct Global {
    ValueType type;                         // Type of the global
    bool is_mutable;                        // Mutability flag
    std::vector<uint8_t> init_expr;         // Initialization expression bytecode

    Global() : type(ValueType::I32), is_mutable(false) {}
    Global(ValueType t, bool mut, std::vector<uint8_t> init)
        : type(t), is_mutable(mut), init_expr(std::move(init)) {}
};

/**
 * Represents a table for function references.
 */
struct Table {
    ValueType element_type;                 // Type of elements (funcref in MVP)
    Limits limits;                          // Min/max elements

    Table() : element_type(ValueType::I32) {}
    Table(ValueType type, Limits l) : element_type(type), limits(std::move(l)) {}
};

/**
 * External kind for imports and exports.
 */
enum class ExternalKind : uint8_t {
    FUNCTION = 0x00,
    TABLE = 0x01,
    MEMORY = 0x02,
    GLOBAL = 0x03
};

/**
 * Represents an export from the module.
 */
struct Export {
    std::string name;                       // Export name
    ExternalKind kind;                      // What is being exported
    uint32_t index;                         // Index into the respective space

    Export() : kind(ExternalKind::FUNCTION), index(0) {}
    Export(std::string n, ExternalKind k, uint32_t idx)
        : name(std::move(n)), kind(k), index(idx) {}
};

/**
 * Represents an import into the module.
 */
struct Import {
    std::string module_name;                // Module name to import from
    std::string field_name;                 // Field name to import
    ExternalKind kind;                      // What is being imported

    // Type-specific data (only one is used based on kind)
    uint32_t type_index;                    // For functions
    MemoryType memory;                      // For memory
    Table table;                            // For tables
    Global global;                          // For globals

    Import() : kind(ExternalKind::FUNCTION), type_index(0) {}
};

/**
 * Represents a data segment for initializing memory.
 */
struct DataSegment {
    uint32_t memory_index;                  // Memory index (always 0 in MVP)
    std::vector<uint8_t> offset_expr;       // Offset expression bytecode
    std::vector<uint8_t> data;              // Raw data bytes

    DataSegment() : memory_index(0) {}
};

/**
 * Represents an element segment for initializing tables.
 */
struct ElementSegment {
    uint32_t table_index;                   // Table index (always 0 in MVP)
    std::vector<uint8_t> offset_expr;       // Offset expression bytecode
    std::vector<uint32_t> func_indices;     // Function indices

    ElementSegment() : table_index(0) {}
};

/**
 * Represents a complete WebAssembly module.
 * Contains all sections parsed from the binary format.
 */
class Module {
public:
    Module() = default;
    ~Module() = default;

    // Section data
    std::vector<FuncType> types;            // Type section
    std::vector<Function> functions;        // Code section (combined with function section)
    std::vector<uint32_t> function_types;   // Function section (type indices)
    std::vector<MemoryType> memories;       // Memory section
    std::vector<Global> globals;            // Global section
    std::vector<Table> tables;              // Table section
    std::vector<Export> exports;            // Export section
    std::vector<Import> imports;            // Import section
    std::vector<DataSegment> data_segments; // Data section
    std::vector<ElementSegment> element_segments; // Element section

    uint32_t start_function_index;          // Start section (optional)
    bool has_start_function;

    Module(const Module&) = delete;
    Module& operator=(const Module&) = delete;
    Module(Module&&) = default;
    Module& operator=(Module&&) = default;

    // Query methods
    const FuncType* getFunctionType(uint32_t func_index) const;
    const Export* findExport(const std::string& name) const;
    uint32_t getImportedFunctionCount() const;
    uint32_t getTotalFunctionCount() const;
};

} // namespace wasm

#endif // WASM_MODULE_H
