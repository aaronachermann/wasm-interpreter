#include "../include/decoder.h"
#include "../include/interpreter.h"
#include "../include/memory.h"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

/**
 * Unified test runner for all WebAssembly test suites.
 * Runs all 167 tests across three test modules.
 *
 * Usage: ./run_all_tests
 *
 * Returns:
 *   0 - All tests passed
 *   1 - Some tests failed
 */

// ANSI color codes
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_BOLD "\033[1m"
#define COLOR_RESET "\033[0m"

struct TestCase {
    std::string name;
};

class TestSuite {
public:
    TestSuite(const std::string& name, const std::string& file)
        : suite_name_(name), wasm_file_(file), passed_(0), failed_(0) {}

    void addTest(const std::string& test_name) {
        tests_.push_back({test_name});
    }

    bool run() {
        std::cout << COLOR_BOLD << COLOR_BLUE
                  << "\n=== Test Suite: " << suite_name_ << " ==="
                  << COLOR_RESET << "\n";
        std::cout << "File: " << wasm_file_ << "\n";
        std::cout << "Tests: " << tests_.size() << "\n\n";

        try {
            // Load and instantiate module
            wasm::Decoder decoder;
            wasm::Module module = decoder.parse(wasm_file_);

            wasm::Interpreter interpreter;
            interpreter.instantiate(std::move(module));

            // Run all tests in this suite
            for (const auto& test : tests_) {
                try {
                    interpreter.call(test.name);
                    passed_++;
                    std::cout << COLOR_GREEN << "✓ " << COLOR_RESET
                              << std::left << std::setw(40) << test.name
                              << " - PASSED\n";
                } catch (const std::exception& e) {
                    failed_++;
                    failed_tests_.push_back(test.name);
                    std::cout << COLOR_RED << "✗ " << COLOR_RESET
                              << std::left << std::setw(40) << test.name
                              << " - FAILED: " << e.what() << "\n";
                }
            }

            // Print suite summary
            std::cout << "\nSuite Results:\n";
            std::cout << "  Total:  " << tests_.size() << "\n";
            std::cout << COLOR_GREEN << "  Passed: " << passed_ << COLOR_RESET << "\n";
            if (failed_ > 0) {
                std::cout << COLOR_RED << "  Failed: " << failed_ << COLOR_RESET << "\n";
            }

            return failed_ == 0;

        } catch (const std::exception& e) {
            std::cerr << COLOR_RED << "✗ Failed to load module: " << e.what()
                      << COLOR_RESET << "\n";
            return false;
        }
    }

    int getPassed() const { return passed_; }
    int getFailed() const { return failed_; }
    const std::vector<std::string>& getFailedTests() const { return failed_tests_; }

private:
    std::string suite_name_;
    std::string wasm_file_;
    std::vector<TestCase> tests_;
    int passed_;
    int failed_;
    std::vector<std::string> failed_tests_;
};

int main() {
    std::cout << COLOR_BOLD
              << "==========================================\n"
              << "WebAssembly Interpreter - Complete Test Suite\n"
              << "NVIDIA Engineering Assessment\n"
              << "==========================================\n"
              << COLOR_RESET << "\n";

    // Test Suite 01 - i32 Operations and Control Flow
    TestSuite suite01("i32 Operations & Control Flow", "tests/wat/01_test.wasm");

    // Basic arithmetic
    suite01.addTest("_test_store");
    suite01.addTest("_test_addition");
    suite01.addTest("_test_subtraction");
    suite01.addTest("_test_multiplication");
    suite01.addTest("_test_division_signed");
    suite01.addTest("_test_division_unsigned");
    suite01.addTest("_test_remainder");

    // Bitwise operations
    suite01.addTest("_test_and");
    suite01.addTest("_test_or");
    suite01.addTest("_test_xor");
    suite01.addTest("_test_shift_left");
    suite01.addTest("_test_shift_right_signed");
    suite01.addTest("_test_shift_right_unsigned");

    // Memory operations
    suite01.addTest("_test_store_load");
    suite01.addTest("_test_store_load_byte_unsigned");
    suite01.addTest("_test_store_load_byte_signed");

    // Local variables
    suite01.addTest("_test_locals_arithmetic");
    suite01.addTest("_test_locals_tee");

    // Global variables
    suite01.addTest("_test_global_increment");
    suite01.addTest("_test_global_constant");
    suite01.addTest("_test_global_multiple");

    // Combined operations
    suite01.addTest("_test_combined");

    // Comparisons
    suite01.addTest("_test_eq");
    suite01.addTest("_test_ne");
    suite01.addTest("_test_lt_s");
    suite01.addTest("_test_lt_u");
    suite01.addTest("_test_gt_s");
    suite01.addTest("_test_gt_u");
    suite01.addTest("_test_le_s");
    suite01.addTest("_test_ge_s");
    suite01.addTest("_test_eqz_zero");
    suite01.addTest("_test_eqz_nonzero");

    // Unary operations
    suite01.addTest("_test_clz");
    suite01.addTest("_test_ctz");
    suite01.addTest("_test_popcnt");
    suite01.addTest("_test_popcnt_all");

    // Rotate operations
    suite01.addTest("_test_rotl");
    suite01.addTest("_test_rotr");
    suite01.addTest("_test_rotl_wrap");

    // 16-bit memory operations
    suite01.addTest("_test_load16_u");
    suite01.addTest("_test_load16_s");
    suite01.addTest("_test_load16_32768");

    // Control flow - select
    suite01.addTest("_test_select_true");
    suite01.addTest("_test_select_false");

    // Control flow - if/else
    suite01.addTest("_test_if_true");
    suite01.addTest("_test_if_false");
    suite01.addTest("_test_if_no_else");
    suite01.addTest("_test_nested_if");

    // Control flow - blocks
    suite01.addTest("_test_block_break");
    suite01.addTest("_test_block_no_break");

    // Control flow - loops
    suite01.addTest("_test_loop_sum");
    suite01.addTest("_test_loop_early_break");

    // Control flow - br_table
    suite01.addTest("_test_br_table_case0");
    suite01.addTest("_test_br_table_case2");

    // Test Suite 02 - Floats, Recursion, and Type Conversions
    TestSuite suite02("Floats, Recursion & Type Conversions", "tests/wat/02_test_prio1.wasm");

    // Function calls and recursion
    suite02.addTest("_test_call_add");
    suite02.addTest("_test_call_composition");
    suite02.addTest("_test_call_square");
    suite02.addTest("_test_call_multiple");
    suite02.addTest("_test_return_early_true");
    suite02.addTest("_test_return_early_false");
    suite02.addTest("_test_abs_negative");
    suite02.addTest("_test_abs_positive");
    suite02.addTest("_test_factorial");
    suite02.addTest("_test_fibonacci");

    // F32 operations
    suite02.addTest("_test_f32_add");
    suite02.addTest("_test_f32_sub");
    suite02.addTest("_test_f32_mul");
    suite02.addTest("_test_f32_div");
    suite02.addTest("_test_f32_min");
    suite02.addTest("_test_f32_max");
    suite02.addTest("_test_f32_abs");
    suite02.addTest("_test_f32_neg");
    suite02.addTest("_test_f32_sqrt");
    suite02.addTest("_test_f32_ceil");
    suite02.addTest("_test_f32_floor");
    suite02.addTest("_test_f32_trunc");
    suite02.addTest("_test_f32_nearest");

    // F32 comparisons
    suite02.addTest("_test_f32_eq");
    suite02.addTest("_test_f32_ne");
    suite02.addTest("_test_f32_lt");
    suite02.addTest("_test_f32_gt");
    suite02.addTest("_test_f32_le");
    suite02.addTest("_test_f32_ge");
    suite02.addTest("_test_f32_call");

    // F64 operations
    suite02.addTest("_test_f64_add");
    suite02.addTest("_test_f64_mul");
    suite02.addTest("_test_f64_sqrt");
    suite02.addTest("_test_f64_gt");

    // Type conversions
    suite02.addTest("_test_convert_i32_to_f32_s");
    suite02.addTest("_test_convert_i32_to_f32_u");
    suite02.addTest("_test_convert_f32_to_i32_s");
    suite02.addTest("_test_convert_f32_to_i32_u");
    suite02.addTest("_test_convert_i32_to_f64_s");
    suite02.addTest("_test_convert_f64_to_i32_s");
    suite02.addTest("_test_promote_f32_to_f64");
    suite02.addTest("_test_demote_f64_to_f32");
    suite02.addTest("_test_reinterpret_f32_to_i32");
    suite02.addTest("_test_reinterpret_i32_to_f32");

    // Parametric operations
    suite02.addTest("_test_drop_simple");
    suite02.addTest("_test_drop_multiple");
    suite02.addTest("_test_nop");
    suite02.addTest("_test_drop_in_computation");

    // Memory operations
    suite02.addTest("_test_memory_size");
    suite02.addTest("_test_memory_grow");
    suite02.addTest("_test_memory_size_after_grow");
    suite02.addTest("_test_memory_grow_multiple");
    suite02.addTest("_test_memory_write_grown");

    // Combined tests
    suite02.addTest("_test_combined_functions");
    suite02.addTest("_test_combined_float_convert");

    // Test Suite 03 - i64 Operations, Data Segments, and call_indirect
    TestSuite suite03("i64 Operations, Data Segments & Tables", "tests/wat/03_test_prio2.wasm");

    // Data segment tests
    suite03.addTest("_test_data_read_char_h");
    suite03.addTest("_test_data_read_char_e");
    suite03.addTest("_test_data_read_i32_42");
    suite03.addTest("_test_data_read_i32_255");
    suite03.addTest("_test_data_read_char_t");
    suite03.addTest("_test_data_read_exclaim");

    // Call indirect tests
    suite03.addTest("_test_call_indirect_add");
    suite03.addTest("_test_call_indirect_sub");
    suite03.addTest("_test_call_indirect_mul");
    suite03.addTest("_test_call_indirect_div");
    suite03.addTest("_test_call_indirect_dynamic");
    suite03.addTest("_test_call_indirect_loop");

    // i64 arithmetic tests
    suite03.addTest("_test_i64_add");
    suite03.addTest("_test_i64_sub");
    suite03.addTest("_test_i64_mul");
    suite03.addTest("_test_i64_div_s");
    suite03.addTest("_test_i64_div_u");
    suite03.addTest("_test_i64_rem_s");

    // i64 bitwise tests
    suite03.addTest("_test_i64_and");
    suite03.addTest("_test_i64_or");
    suite03.addTest("_test_i64_xor");
    suite03.addTest("_test_i64_shl");
    suite03.addTest("_test_i64_shr_s");
    suite03.addTest("_test_i64_shr_u");
    suite03.addTest("_test_i64_rotl");
    suite03.addTest("_test_i64_rotr");

    // i64 unary tests
    suite03.addTest("_test_i64_clz");
    suite03.addTest("_test_i64_ctz");
    suite03.addTest("_test_i64_popcnt");

    // i64 comparison tests
    suite03.addTest("_test_i64_eq");
    suite03.addTest("_test_i64_ne");
    suite03.addTest("_test_i64_lt_s");
    suite03.addTest("_test_i64_gt_s");
    suite03.addTest("_test_i64_eqz");

    // i64 conversion tests
    suite03.addTest("_test_i64_extend_i32_s");
    suite03.addTest("_test_i64_extend_i32_u");
    suite03.addTest("_test_i64_wrap");
    suite03.addTest("_test_i64_trunc_f32_s");
    suite03.addTest("_test_i64_trunc_f64_s");
    suite03.addTest("_test_i64_convert_to_f32");
    suite03.addTest("_test_i64_convert_to_f64");

    // i64 memory tests
    suite03.addTest("_test_i64_store_load");
    suite03.addTest("_test_i64_load32_u");
    suite03.addTest("_test_i64_load32_s");

    // i64 function tests
    suite03.addTest("_test_i64_call_function");
    suite03.addTest("_test_i64_large_mul");
    suite03.addTest("_test_i64_bit_pattern");

    // Trap tests
    suite03.addTest("_test_trap_safe_div");
    suite03.addTest("_test_trap_divisor_zero");
    suite03.addTest("_test_trap_check_div_zero");
    suite03.addTest("_test_trap_check_mem_valid");
    suite03.addTest("_test_trap_check_mem_invalid");
    suite03.addTest("_test_trap_check_overflow");
    suite03.addTest("_test_trap_check_rem_zero");
    suite03.addTest("_test_trap_check_i64_div_zero");

    // Combined tests
    suite03.addTest("_test_combined_data_i64");
    suite03.addTest("_test_combined_indirect_i64");
    suite03.addTest("_test_combined_all_features");

    // Run all test suites
    bool suite01_pass = suite01.run();
    bool suite02_pass = suite02.run();
    bool suite03_pass = suite03.run();

    // Calculate overall statistics
    int total_passed = suite01.getPassed() + suite02.getPassed() + suite03.getPassed();
    int total_failed = suite01.getFailed() + suite02.getFailed() + suite03.getFailed();
    int total_tests = total_passed + total_failed;

    // Print comprehensive summary
    std::cout << "\n" << COLOR_BOLD
              << "==========================================\n"
              << "Comprehensive Test Summary\n"
              << "==========================================\n"
              << COLOR_RESET;

    std::cout << "\nOverall Results:\n";
    std::cout << "  Total Tests:     " << total_tests << "\n";
    std::cout << COLOR_GREEN << "  Total Passed:    " << total_passed << COLOR_RESET << "\n";

    if (total_failed > 0) {
        std::cout << COLOR_RED << "  Total Failed:    " << total_failed << COLOR_RESET << "\n";
    } else {
        std::cout << "  Total Failed:    " << total_failed << "\n";
    }

    // Calculate and display pass rate
    if (total_tests > 0) {
        double pass_rate = (static_cast<double>(total_passed) / total_tests) * 100.0;
        std::cout << "\n  " << COLOR_BOLD << "Pass Rate:       "
                  << std::fixed << std::setprecision(1) << pass_rate << "%"
                  << COLOR_RESET << "\n";
    }

    // Show failed tests if any
    if (total_failed > 0) {
        std::cout << "\n" << COLOR_RED << COLOR_BOLD << "Failed Tests:" << COLOR_RESET << "\n";

        if (suite01.getFailed() > 0) {
            std::cout << "\n  Suite 01:\n";
            for (const auto& test : suite01.getFailedTests()) {
                std::cout << COLOR_RED << "    - " << test << COLOR_RESET << "\n";
            }
        }

        if (suite02.getFailed() > 0) {
            std::cout << "\n  Suite 02:\n";
            for (const auto& test : suite02.getFailedTests()) {
                std::cout << COLOR_RED << "    - " << test << COLOR_RESET << "\n";
            }
        }

        if (suite03.getFailed() > 0) {
            std::cout << "\n  Suite 03:\n";
            for (const auto& test : suite03.getFailedTests()) {
                std::cout << COLOR_RED << "    - " << test << COLOR_RESET << "\n";
            }
        }

        std::cout << "\n" << COLOR_YELLOW
                  << "Some tests failed. Review output above for details."
                  << COLOR_RESET << "\n\n";

        std::cout << COLOR_BOLD
                  << "==========================================\n"
                  << COLOR_RESET;
        return 1;
    }

    // All tests passed
    std::cout << "\n" << COLOR_GREEN << COLOR_BOLD
              << "🎉 All " << total_tests << " tests PASSED! 🎉\n"
              << COLOR_RESET << "\n";

    std::cout << COLOR_GREEN
              << "WebAssembly interpreter is fully functional.\n"
              << "Ready for NVIDIA engineering assessment.\n"
              << COLOR_RESET << "\n";

    std::cout << COLOR_BOLD
              << "==========================================\n"
              << COLOR_RESET;

    return 0;
}
