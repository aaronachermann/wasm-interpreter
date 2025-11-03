#include "stack.h"
#include <iostream>
#include <iomanip>

namespace wasm {

// Stack implementation

void Stack::pushI32(int32_t value) {
    stack_.push_back(TypedValue::makeI32(value));
}

void Stack::pushI64(int64_t value) {
    stack_.push_back(TypedValue::makeI64(value));
}

void Stack::pushF32(float value) {
    stack_.push_back(TypedValue::makeF32(value));
}

void Stack::pushF64(double value) {
    stack_.push_back(TypedValue::makeF64(value));
}

void Stack::push(const TypedValue& value) {
    stack_.push_back(value);
}

int32_t Stack::popI32() {
    checkNotEmpty();
    checkType(ValueType::I32);
    TypedValue val = stack_.back();
    stack_.pop_back();
    return val.value.i32;
}

int64_t Stack::popI64() {
    checkNotEmpty();
    checkType(ValueType::I64);
    TypedValue val = stack_.back();
    stack_.pop_back();
    return val.value.i64;
}

float Stack::popF32() {
    checkNotEmpty();
    checkType(ValueType::F32);
    TypedValue val = stack_.back();
    stack_.pop_back();
    return val.value.f32;
}

double Stack::popF64() {
    checkNotEmpty();
    checkType(ValueType::F64);
    TypedValue val = stack_.back();
    stack_.pop_back();
    return val.value.f64;
}

TypedValue Stack::pop() {
    checkNotEmpty();
    TypedValue val = stack_.back();
    stack_.pop_back();
    return val;
}

int32_t Stack::peekI32() const {
    checkNotEmpty();
    checkType(ValueType::I32);
    return stack_.back().value.i32;
}

int64_t Stack::peekI64() const {
    checkNotEmpty();
    checkType(ValueType::I64);
    return stack_.back().value.i64;
}

float Stack::peekF32() const {
    checkNotEmpty();
    checkType(ValueType::F32);
    return stack_.back().value.f32;
}

double Stack::peekF64() const {
    checkNotEmpty();
    checkType(ValueType::F64);
    return stack_.back().value.f64;
}

const TypedValue& Stack::peek() const {
    checkNotEmpty();
    return stack_.back();
}

const TypedValue& Stack::peek(size_t depth) const {
    checkDepth(depth);
    return stack_[stack_.size() - 1 - depth];
}

void Stack::checkNotEmpty() const {
    if (stack_.empty()) {
        throw StackError("Stack underflow");
    }
}

void Stack::checkType(ValueType expected) const {
    if (stack_.back().type != expected) {
        throw StackError("Type mismatch: expected " + valueTypeToString(expected) +
                        ", got " + valueTypeToString(stack_.back().type));
    }
}

void Stack::checkDepth(size_t depth) const {
    if (depth >= stack_.size()) {
        throw StackError("Invalid stack depth");
    }
}

void Stack::dump() const {
    std::cout << "Stack (size=" << stack_.size() << "):" << std::endl;
    for (size_t i = 0; i < stack_.size(); i++) {
        const auto& val = stack_[i];
        std::cout << "  [" << i << "] " << valueTypeToString(val.type) << ": ";

        switch (val.type) {
            case ValueType::I32:
                std::cout << val.value.i32;
                break;
            case ValueType::I64:
                std::cout << val.value.i64;
                break;
            case ValueType::F32:
                std::cout << val.value.f32;
                break;
            case ValueType::F64:
                std::cout << val.value.f64;
                break;
            default:
                std::cout << "?";
        }
        std::cout << std::endl;
    }
}

// CallStack implementation

void CallStack::push(const CallFrame& frame) {
    checkDepthLimit();
    frames_.push_back(frame);
}

CallFrame CallStack::pop() {
    checkNotEmpty();
    CallFrame frame = frames_.back();
    frames_.pop_back();
    return frame;
}

const CallFrame& CallStack::top() const {
    checkNotEmpty();
    return frames_.back();
}

CallFrame& CallStack::top() {
    checkNotEmpty();
    return frames_.back();
}

void CallStack::checkNotEmpty() const {
    if (frames_.empty()) {
        throw StackError("Call stack underflow");
    }
}

void CallStack::checkDepthLimit() const {
    if (frames_.size() >= MAX_DEPTH) {
        throw StackError("Call stack overflow: maximum depth exceeded");
    }
}

} // namespace wasm
