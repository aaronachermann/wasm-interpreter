#ifndef WASM_TYPES_H
#define WASM_TYPES_H

#include <cstdint>
#include <vector>
#include <string>

namespace wasm {

/**
 * WebAssembly value types as defined in the MVP specification.
 */
enum class ValueType : uint8_t {
    I32 = 0x7F,  // 32-bit integer
    I64 = 0x7E,  // 64-bit integer
    F32 = 0x7D,  // 32-bit floating point
    F64 = 0x7C,  // 64-bit floating point
    VOID = 0x40  // Empty type for blocks/functions with no result
};

/**
 * Union representing a WebAssembly value.
 * Only one field is active at a time based on the associated ValueType.
 */
union Value {
    int32_t i32;
    int64_t i64;
    float f32;
    double f64;

    Value() : i64(0) {}
    explicit Value(int32_t v) : i32(v) {}
    explicit Value(int64_t v) : i64(v) {}
    explicit Value(float v) : f32(v) {}
    explicit Value(double v) : f64(v) {}
};

/**
 * Typed value combining a type tag with its value.
 */
struct TypedValue {
    ValueType type;
    Value value;

    TypedValue() : type(ValueType::I32), value() {}
    TypedValue(ValueType t, Value v) : type(t), value(v) {}

    // Type-specific constructors
    static TypedValue makeI32(int32_t v) {
        return TypedValue(ValueType::I32, Value(v));
    }

    static TypedValue makeI64(int64_t v) {
        return TypedValue(ValueType::I64, Value(v));
    }

    static TypedValue makeF32(float v) {
        return TypedValue(ValueType::F32, Value(v));
    }

    static TypedValue makeF64(double v) {
        return TypedValue(ValueType::F64, Value(v));
    }
};

/**
 * Function type signature containing parameter and result types.
 */
struct FuncType {
    std::vector<ValueType> params;   // Parameter types
    std::vector<ValueType> results;  // Result types (MVP supports 0 or 1)

    FuncType() = default;
    FuncType(std::vector<ValueType> p, std::vector<ValueType> r)
        : params(std::move(p)), results(std::move(r)) {}

    bool operator==(const FuncType& other) const {
        return params == other.params && results == other.results;
    }
};

/**
 * Limits for memory and tables.
 */
struct Limits {
    uint32_t min;              // Minimum size
    uint32_t max;              // Maximum size (0 means unbounded)
    bool has_max;              // Whether maximum is specified

    Limits() : min(0), max(0), has_max(false) {}
    Limits(uint32_t minimum) : min(minimum), max(0), has_max(false) {}
    Limits(uint32_t minimum, uint32_t maximum)
        : min(minimum), max(maximum), has_max(true) {}
};

/**
 * Utility functions for type operations.
 */
std::string valueTypeToString(ValueType type);
size_t getValueTypeSize(ValueType type);

} // namespace wasm

#endif // WASM_TYPES_H
