#include "module.h"

namespace wasm {

const FuncType* Module::getFunctionType(uint32_t func_index) const {
    // Adjust for imports
    uint32_t import_count = getImportedFunctionCount();

    if (func_index < import_count) {
        // Handle imported functions
        // Import type lookup is handled in Interpreter::execute() where imports are routed
        // WASI and other imported functions have their own type checking mechanisms
        return nullptr;
    }

    uint32_t local_index = func_index - import_count;
    if (local_index >= function_types.size()) {
        return nullptr;
    }

    uint32_t type_index = function_types[local_index];
    if (type_index >= types.size()) {
        return nullptr;
    }

    return &types[type_index];
}

const Export* Module::findExport(const std::string& name) const {
    for (const auto& exp : exports) {
        if (exp.name == name) {
            return &exp;
        }
    }
    return nullptr;
}

uint32_t Module::getImportedFunctionCount() const {
    uint32_t count = 0;
    for (const auto& import : imports) {
        if (import.kind == ExternalKind::FUNCTION) {
            count++;
        }
    }
    return count;
}

uint32_t Module::getTotalFunctionCount() const {
    return getImportedFunctionCount() + static_cast<uint32_t>(functions.size());
}

} // namespace wasm
