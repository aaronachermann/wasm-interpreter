#include "instructions.h"
#include <unordered_map>

namespace wasm {

namespace {
    // Opcode name lookup table
    const std::unordered_map<Opcode, std::string> opcode_names = {
        // Control flow
        {Opcode::UNREACHABLE, "unreachable"},
        {Opcode::NOP, "nop"},
        {Opcode::BLOCK, "block"},
        {Opcode::LOOP, "loop"},
        {Opcode::IF, "if"},
        {Opcode::ELSE, "else"},
        {Opcode::END, "end"},
        {Opcode::BR, "br"},
        {Opcode::BR_IF, "br_if"},
        {Opcode::BR_TABLE, "br_table"},
        {Opcode::RETURN, "return"},
        {Opcode::CALL, "call"},
        {Opcode::CALL_INDIRECT, "call_indirect"},

        // Parametric
        {Opcode::DROP, "drop"},
        {Opcode::SELECT, "select"},

        // Variable access
        {Opcode::LOCAL_GET, "local.get"},
        {Opcode::LOCAL_SET, "local.set"},
        {Opcode::LOCAL_TEE, "local.tee"},
        {Opcode::GLOBAL_GET, "global.get"},
        {Opcode::GLOBAL_SET, "global.set"},

        // Memory
        {Opcode::I32_LOAD, "i32.load"},
        {Opcode::I64_LOAD, "i64.load"},
        {Opcode::F32_LOAD, "f32.load"},
        {Opcode::F64_LOAD, "f64.load"},
        {Opcode::I32_STORE, "i32.store"},
        {Opcode::I64_STORE, "i64.store"},
        {Opcode::F32_STORE, "f32.store"},
        {Opcode::F64_STORE, "f64.store"},
        {Opcode::MEMORY_SIZE, "memory.size"},
        {Opcode::MEMORY_GROW, "memory.grow"},

        // Constants
        {Opcode::I32_CONST, "i32.const"},
        {Opcode::I64_CONST, "i64.const"},
        {Opcode::F32_CONST, "f32.const"},
        {Opcode::F64_CONST, "f64.const"},

        // i32 operations
        {Opcode::I32_ADD, "i32.add"},
        {Opcode::I32_SUB, "i32.sub"},
        {Opcode::I32_MUL, "i32.mul"},
        {Opcode::I32_DIV_S, "i32.div_s"},
        {Opcode::I32_DIV_U, "i32.div_u"},
        {Opcode::I32_EQ, "i32.eq"},
        {Opcode::I32_NE, "i32.ne"},
        {Opcode::I32_LT_S, "i32.lt_s"},
        {Opcode::I32_LT_U, "i32.lt_u"},
        {Opcode::I32_GT_S, "i32.gt_s"},
        {Opcode::I32_GT_U, "i32.gt_u"},

        // i64 operations
        {Opcode::I64_ADD, "i64.add"},
        {Opcode::I64_SUB, "i64.sub"},
        {Opcode::I64_MUL, "i64.mul"},
        {Opcode::I64_EQ, "i64.eq"},

        // f32 operations
        {Opcode::F32_ADD, "f32.add"},
        {Opcode::F32_SUB, "f32.sub"},
        {Opcode::F32_MUL, "f32.mul"},
        {Opcode::F32_DIV, "f32.div"},

        // f64 operations
        {Opcode::F64_ADD, "f64.add"},
        {Opcode::F64_SUB, "f64.sub"},
        {Opcode::F64_MUL, "f64.mul"},
        {Opcode::F64_DIV, "f64.div"},
    };
}

std::string opcodeToString(Opcode opcode) {
    auto it = opcode_names.find(opcode);
    if (it != opcode_names.end()) {
        return it->second;
    }
    return "unknown";
}

bool isControlFlowInstruction(Opcode opcode) {
    return opcode >= Opcode::UNREACHABLE && opcode <= Opcode::CALL_INDIRECT;
}

bool isMemoryInstruction(Opcode opcode) {
    return (opcode >= Opcode::I32_LOAD && opcode <= Opcode::I64_STORE32) ||
           opcode == Opcode::MEMORY_SIZE || opcode == Opcode::MEMORY_GROW;
}

bool isNumericInstruction(Opcode opcode) {
    return (opcode >= Opcode::I32_CONST && opcode <= Opcode::F64_CONST) ||
           (opcode >= Opcode::I32_EQZ && opcode <= Opcode::F64_REINTERPRET_I64);
}

} // namespace wasm
