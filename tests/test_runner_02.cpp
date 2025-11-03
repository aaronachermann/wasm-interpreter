#include "../include/decoder.h"
#include "../include/interpreter.h"
#include "../include/memory.h"
#include <iostream>
#include <vector>
#include <string>

/**
 * Comprehensive test runner for 02_test_prio1.wasm.
 * Tests function calls, recursion, float operations, and type conversions.
 */

struct TestCase {
    std::string name;
};

int main() {
    try {
        std::cout << "=== WebAssembly Interpreter Test Runner (Priority 1) ===\n\n";

        // Load and instantiate module
        wasm::Decoder decoder;
        wasm::Module module = decoder.parse("tests/wat/02_test_prio1.wasm");

        std::cout << "Module loaded: " << module.functions.size() << " functions\n";
        std::cout << "Globals: " << module.globals.size() << "\n";
        std::cout << "Exports: " << module.exports.size() << "\n\n";

        wasm::Interpreter interpreter;
        interpreter.instantiate(std::move(module));

        std::cout << "Module instantiated successfully\n\n";

        // Define test cases
        std::vector<TestCase> tests = {
            // Function calls and recursion
            {"_test_call_add"},
            {"_test_call_composition"},
            {"_test_call_square"},
            {"_test_call_multiple"},
            {"_test_return_early_true"},
            {"_test_return_early_false"},
            {"_test_abs_negative"},
            {"_test_abs_positive"},
            {"_test_factorial"},
            {"_test_fibonacci"},

            // F32 operations
            {"_test_f32_add"},
            {"_test_f32_sub"},
            {"_test_f32_mul"},
            {"_test_f32_div"},
            {"_test_f32_min"},
            {"_test_f32_max"},
            {"_test_f32_abs"},
            {"_test_f32_neg"},
            {"_test_f32_sqrt"},
            {"_test_f32_ceil"},
            {"_test_f32_floor"},
            {"_test_f32_trunc"},
            {"_test_f32_nearest"},

            // F32 comparisons
            {"_test_f32_eq"},
            {"_test_f32_ne"},
            {"_test_f32_lt"},
            {"_test_f32_gt"},
            {"_test_f32_le"},
            {"_test_f32_ge"},
            {"_test_f32_call"},

            // F64 operations
            {"_test_f64_add"},
            {"_test_f64_mul"},
            {"_test_f64_sqrt"},
            {"_test_f64_gt"},

            // Type conversions
            {"_test_convert_i32_to_f32_s"},
            {"_test_convert_i32_to_f32_u"},
            {"_test_convert_f32_to_i32_s"},
            {"_test_convert_f32_to_i32_u"},
            {"_test_convert_i32_to_f64_s"},
            {"_test_convert_f64_to_i32_s"},
            {"_test_promote_f32_to_f64"},
            {"_test_demote_f64_to_f32"},
            {"_test_reinterpret_f32_to_i32"},
            {"_test_reinterpret_i32_to_f32"},

            // Parametric operations
            {"_test_drop_simple"},
            {"_test_drop_multiple"},
            {"_test_nop"},
            {"_test_drop_in_computation"},

            // Memory operations
            {"_test_memory_size"},
            {"_test_memory_grow"},
            {"_test_memory_size_after_grow"},
            {"_test_memory_grow_multiple"},
            {"_test_memory_write_grown"},

            // Combined tests
            {"_test_combined_functions"},
            {"_test_combined_float_convert"},
        };

        // Run tests
        int passed = 0;
        int failed = 0;

        for (const auto& test : tests) {
            try {
                // Call test function
                interpreter.call(test.name);
                passed++;
                std::cout << "✓ " << test.name << " - PASSED\n";

            } catch (const std::exception& e) {
                failed++;
                std::cout << "✗ " << test.name << " - FAILED: " << e.what() << "\n";
            }
        }

        std::cout << "\n=== Test Results ===\n";
        std::cout << "Total: " << tests.size() << "\n";
        std::cout << "Passed: " << passed << "\n";
        std::cout << "Failed: " << failed << "\n";

        if (failed == 0) {
            std::cout << "\n🎉 All tests PASSED!\n";
            return 0;
        } else {
            std::cout << "\n❌ Some tests failed\n";
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
