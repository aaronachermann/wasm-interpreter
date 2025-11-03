#ifndef WASM_STACK_H
#define WASM_STACK_H

#include "types.h"
#include <vector>
#include <stdexcept>
#include <cstdint>

namespace wasm {

/**
 * Exception thrown on stack errors.
 */
class StackError : public std::runtime_error {
public:
    explicit StackError(const std::string& message)
        : std::runtime_error("Stack error: " + message) {}
};

/**
 * Execution stack for WebAssembly values.
 * Provides type-safe push/pop operations with runtime type checking.
 */
class Stack {
public:
    Stack() = default;
    ~Stack() = default;

    // Push operations
    void pushI32(int32_t value);
    void pushI64(int64_t value);
    void pushF32(float value);
    void pushF64(double value);
    void push(const TypedValue& value);

    // Pop operations with type checking
    int32_t popI32();
    int64_t popI64();
    float popF32();
    double popF64();
    TypedValue pop();

    // Peek operations (non-destructive)
    int32_t peekI32() const;
    int64_t peekI64() const;
    float peekF32() const;
    double peekF64() const;
    const TypedValue& peek() const;
    const TypedValue& peek(size_t depth) const;

    // Stack state
    size_t size() const { return stack_.size(); }
    bool empty() const { return stack_.empty(); }
    void clear() { stack_.clear(); }

    // For debugging
    void dump() const;

private:
    std::vector<TypedValue> stack_;

    void checkNotEmpty() const;
    void checkType(ValueType expected) const;
    void checkDepth(size_t depth) const;
};

/**
 * Call frame for function invocations.
 * Tracks return address and local variable base pointer.
 */
struct CallFrame {
    uint32_t function_index;        // Index of the called function
    size_t return_pc;               // Program counter to return to
    size_t locals_base;             // Base index for local variables in stack
    size_t stack_base;              // Base of operand stack for this frame

    CallFrame(uint32_t func_idx, size_t ret_pc, size_t locals, size_t stack)
        : function_index(func_idx), return_pc(ret_pc),
          locals_base(locals), stack_base(stack) {}
};

/**
 * Call stack for tracking function invocations.
 */
class CallStack {
public:
    CallStack() = default;
    ~CallStack() = default;

    void push(const CallFrame& frame);
    CallFrame pop();
    const CallFrame& top() const;
    CallFrame& top();

    size_t size() const { return frames_.size(); }
    bool empty() const { return frames_.empty(); }
    void clear() { frames_.clear(); }

    // Stack depth limit to prevent infinite recursion
    static constexpr size_t MAX_DEPTH = 1024;

private:
    std::vector<CallFrame> frames_;

    void checkNotEmpty() const;
    void checkDepthLimit() const;
};

} // namespace wasm

#endif // WASM_STACK_H
