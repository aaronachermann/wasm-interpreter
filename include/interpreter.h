#ifndef WASM_INTERPRETER_H
#define WASM_INTERPRETER_H

#include "module.h"
#include "stack.h"
#include "memory.h"
#include "instructions.h"
#include <vector>
#include <memory>
#include <string>
#include <stdexcept>
#include <unordered_map>

namespace wasm {

/**
 * Exception thrown during interpretation.
 */
class InterpreterError : public std::runtime_error {
public:
    explicit InterpreterError(const std::string& message)
        : std::runtime_error("Interpreter error: " + message) {}
};

/**
 * Trap exception for WebAssembly traps (runtime errors).
 */
class Trap : public std::runtime_error {
public:
    explicit Trap(const std::string& message)
        : std::runtime_error("Trap: " + message) {}
};

/**
 * WebAssembly interpreter.
 * Executes WebAssembly bytecode using stack-based interpretation.
 */
class Interpreter {
public:
    Interpreter();
    ~Interpreter();

    /**
     * Instantiate a module, preparing it for execution.
     * This initializes memory, globals, and tables.
     * @param module The module to instantiate
     */
    void instantiate(Module&& module);

    /**
     * Call an exported function by name.
     * @param function_name Name of the exported function
     * @param args Arguments to pass to the function
     * @return Results from the function
     */
    std::vector<TypedValue> call(const std::string& function_name,
                                  const std::vector<TypedValue>& args = {});

    /**
     * Call a function by index.
     * @param func_index Function index
     * @param args Arguments to pass to the function
     * @return Results from the function
     */
    std::vector<TypedValue> callFunction(uint32_t func_index,
                                         const std::vector<TypedValue>& args = {});

    /**
     * Get current state for debugging.
     */
    void dumpState() const;

private:
    std::unique_ptr<Module> module_;
    Stack stack_;
    CallStack call_stack_;
    std::unique_ptr<Memory> memory_;
    std::vector<TypedValue> globals_;
    std::vector<TypedValue> locals_;

    // Execution state
    const uint8_t* code_;           // Current function bytecode
    size_t code_size_;              // Size of current function bytecode
    size_t pc_;                     // Program counter

    // Module initialization
    void initializeMemory();
    void initializeGlobals();
    void initializeTables();
    void initializeElements();
    void initializeData();

    // Execution
    void execute(uint32_t func_index);
    void executeInstruction();
    void executeBlock(ValueType block_type);
    void executeLoop(ValueType block_type);
    void executeIf(ValueType block_type);

    // Control flow helpers
    struct Label {
        size_t target_pc;           // Jump target
        size_t stack_height;        // Stack height at label
        bool is_loop;               // Loop vs block/if
        size_t arity;               // Number of result values (0 or 1 in MVP)
    };
    std::vector<Label> labels_;

    void pushLabel(size_t target_pc, size_t stack_height, bool is_loop, size_t arity = 0);
    void popLabel();
    void branch(uint32_t depth);
    void branchIf(uint32_t depth);
    void branchTable(const std::vector<uint32_t>& targets, uint32_t default_target);

    // Control flow helpers
    size_t findMatchingEnd(size_t start_pc) const;
    size_t findMatchingElseAndEnd(size_t start_pc, size_t& else_pc) const;
    void skipInstructionOperands(uint8_t opcode, size_t& pc) const;

    // WASI support
    std::unordered_map<std::string, bool> wasi_imports_;
    void executeWASIFdWrite();

    // Instruction execution by category
    void executeControlFlow(Opcode opcode);
    void executeParametric(Opcode opcode);
    void executeVariable(Opcode opcode);
    void executeMemory(Opcode opcode);
    void executeNumericConst(Opcode opcode);
    void executeNumeric(Opcode opcode);
    void executeConversion(Opcode opcode);

    // Variable access
    void setLocal(uint32_t index, const TypedValue& value);
    TypedValue getLocal(uint32_t index) const;
    void setGlobal(uint32_t index, const TypedValue& value);
    TypedValue getGlobal(uint32_t index) const;

    // Memory access helpers
    MemArg readMemArg();
    uint32_t effectiveAddress(uint32_t base, uint32_t offset) const;

    // Reading immediates from bytecode
    uint8_t readByte();
    uint32_t readVarUint32();
    int32_t readVarInt32();
    int64_t readVarInt64();
    float readF32();
    double readF64();

    // Validation
    void validateFunctionCall(uint32_t func_index) const;
    void validateMemoryAccess() const;
    void checkStackUnderflow(size_t required) const;
};

} // namespace wasm

#endif // WASM_INTERPRETER_H
