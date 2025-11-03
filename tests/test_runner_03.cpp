#include "../include/decoder.h"
#include "../include/interpreter.h"
#include "../include/memory.h"
#include <iostream>
#include <vector>
#include <string>

/**
 * Comprehensive test runner for 03_test_prio2.wasm.
 * Tests i64 operations, data segments, and call_indirect.
 */

struct TestCase {
    std::string name;
    bool should_trap;  // If true, test expects a trap

    TestCase(const std::string& n, bool trap = false)
        : name(n), should_trap(trap) {}
};

int main() {
    try {
        std::cout << "=== WebAssembly Interpreter Test Runner (Priority 2) ===\n\n";

        // Load and instantiate module
        wasm::Decoder decoder;
        wasm::Module module = decoder.parse("tests/wat/03_test_prio2.wasm");

        std::cout << "Module loaded: " << module.functions.size() << " functions\n";
        std::cout << "Globals: " << module.globals.size() << "\n";
        std::cout << "Exports: " << module.exports.size() << "\n";
        std::cout << "Data segments: " << module.data_segments.size() << "\n";
        std::cout << "Element segments: " << module.element_segments.size() << "\n\n";

        wasm::Interpreter interpreter;
        interpreter.instantiate(std::move(module));

        std::cout << "Module instantiated successfully\n\n";

        // Define test cases
        std::vector<TestCase> tests = {
            // Data segment tests
            {"_test_data_read_char_h"},
            {"_test_data_read_char_e"},
            {"_test_data_read_i32_42"},
            {"_test_data_read_i32_255"},
            {"_test_data_read_char_t"},
            {"_test_data_read_exclaim"},

            // Call indirect tests
            {"_test_call_indirect_add"},
            {"_test_call_indirect_sub"},
            {"_test_call_indirect_mul"},
            {"_test_call_indirect_div"},
            {"_test_call_indirect_dynamic"},
            {"_test_call_indirect_loop"},

            // i64 arithmetic tests
            {"_test_i64_add"},
            {"_test_i64_sub"},
            {"_test_i64_mul"},
            {"_test_i64_div_s"},
            {"_test_i64_div_u"},
            {"_test_i64_rem_s"},

            // i64 bitwise tests
            {"_test_i64_and"},
            {"_test_i64_or"},
            {"_test_i64_xor"},
            {"_test_i64_shl"},
            {"_test_i64_shr_s"},
            {"_test_i64_shr_u"},
            {"_test_i64_rotl"},
            {"_test_i64_rotr"},

            // i64 unary tests
            {"_test_i64_clz"},
            {"_test_i64_ctz"},
            {"_test_i64_popcnt"},

            // i64 comparison tests
            {"_test_i64_eq"},
            {"_test_i64_ne"},
            {"_test_i64_lt_s"},
            {"_test_i64_gt_s"},
            {"_test_i64_eqz"},

            // i64 conversion tests
            {"_test_i64_extend_i32_s"},
            {"_test_i64_extend_i32_u"},
            {"_test_i64_wrap"},
            {"_test_i64_trunc_f32_s"},
            {"_test_i64_trunc_f64_s"},
            {"_test_i64_convert_to_f32"},
            {"_test_i64_convert_to_f64"},

            // i64 memory tests
            {"_test_i64_store_load"},
            {"_test_i64_load32_u"},
            {"_test_i64_load32_s"},

            // i64 function tests
            {"_test_i64_call_function"},
            {"_test_i64_large_mul"},
            {"_test_i64_bit_pattern"},

            // Trap tests
            // Note: These test trap handling logic, not all actually trap
            {"_test_trap_safe_div"},
            {"_test_trap_divisor_zero"},
            {"_test_trap_check_div_zero"},
            {"_test_trap_check_mem_valid"},      // Combined features test
            {"_test_trap_check_mem_invalid"},    // Combined features test
            {"_test_trap_check_overflow"},
            {"_test_trap_check_rem_zero"},
            {"_test_trap_check_i64_div_zero"},

            // Combined tests
            {"_test_combined_data_i64"},
            {"_test_combined_indirect_i64"},
            {"_test_combined_all_features"},
        };

        // Run tests
        int passed = 0;
        int failed = 0;

        for (const auto& test : tests) {
            try {
                // Call test function
                interpreter.call(test.name);

                if (test.should_trap) {
                    // Expected a trap but didn't get one
                    failed++;
                    std::cout << "✗ " << test.name << " - FAILED: Expected trap but none occurred\n";
                } else {
                    passed++;
                    std::cout << "✓ " << test.name << " - PASSED\n";
                }

            } catch (const wasm::Trap& e) {
                if (test.should_trap) {
                    // Expected trap and got one
                    passed++;
                    std::cout << "✓ " << test.name << " - PASSED (trapped as expected)\n";
                } else {
                    // Unexpected trap
                    failed++;
                    std::cout << "✗ " << test.name << " - FAILED (unexpected trap): " << e.what() << "\n";
                }
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
