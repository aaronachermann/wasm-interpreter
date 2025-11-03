#include "types.h"

namespace wasm {

std::string valueTypeToString(ValueType type) {
    switch (type) {
        case ValueType::I32:
            return "i32";
        case ValueType::I64:
            return "i64";
        case ValueType::F32:
            return "f32";
        case ValueType::F64:
            return "f64";
        case ValueType::VOID:
            return "void";
        default:
            return "unknown";
    }
}

size_t getValueTypeSize(ValueType type) {
    switch (type) {
        case ValueType::I32:
        case ValueType::F32:
            return 4;
        case ValueType::I64:
        case ValueType::F64:
            return 8;
        case ValueType::VOID:
            return 0;
        default:
            return 0;
    }
}

} // namespace wasm
