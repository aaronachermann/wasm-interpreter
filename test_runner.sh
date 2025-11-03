#!/bin/bash

# WebAssembly Interpreter Test Runner
# Automated testing suite for NVIDIA engineering assessment
# Runs all 167 WebAssembly tests and generates comprehensive report

set -e

# Colors for terminal output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Directories
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR="$SCRIPT_DIR/build"
BIN_DIR="$BUILD_DIR/bin"

echo -e "${BOLD}=========================================="
echo "WebAssembly Interpreter Test Suite"
echo "NVIDIA Engineering Assessment"
echo -e "==========================================${NC}"
echo ""

# Check if build directory exists, if not create and build
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Build directory not found. Creating...${NC}"
    mkdir -p "$BUILD_DIR"
fi

# Check if test runners exist, build if not
cd "$BUILD_DIR"

if [ ! -f "$BIN_DIR/test_runner" ] || [ ! -f "$BIN_DIR/test_runner_02" ] || [ ! -f "$BIN_DIR/test_runner_03" ]; then
    echo -e "${YELLOW}Building test runners...${NC}"
    cmake .. > /dev/null 2>&1
    cmake --build . > /dev/null 2>&1
    echo -e "${GREEN}✓ Build complete${NC}"
    echo ""
fi

cd "$SCRIPT_DIR"

# Test counters
total_tests=0
passed_tests=0
failed_tests=0

# Arrays to store failed test names
declare -a failed_test_names

# Function to run a test suite and capture results
run_test_suite() {
    local test_runner=$1
    local suite_name=$2
    local test_number=$3

    echo -e "${BOLD}${BLUE}Test Suite ${test_number}: ${suite_name}${NC}"
    echo "--------------------------------------------"

    # Run test and capture output
    local output
    local exit_code

    output=$("$test_runner" 2>&1) || exit_code=$?

    # Parse output for test results
    local suite_passed=0
    local suite_failed=0

    # Extract passed/failed counts from output
    while IFS= read -r line; do
        if [[ $line == "✓"* ]] || [[ $line == *"PASSED"* ]]; then
            echo -e "${GREEN}${line}${NC}"
            ((suite_passed++))
            ((total_tests++))
            ((passed_tests++))
        elif [[ $line == "✗"* ]] || [[ $line == *"FAILED"* ]]; then
            echo -e "${RED}${line}${NC}"
            ((suite_failed++))
            ((total_tests++))
            ((failed_tests++))

            # Extract test name
            test_name=$(echo "$line" | awk '{print $2}')
            failed_test_names+=("$test_name")
        elif [[ $line == *"Test Results"* ]] || [[ $line == *"Total:"* ]] || [[ $line == *"Passed:"* ]] || [[ $line == *"Failed:"* ]]; then
            echo "$line"
        fi
    done <<< "$output"

    echo ""

    return 0
}

# Run all test suites
echo -e "${BOLD}Running WebAssembly Test Suites...${NC}"
echo ""

# Test Suite 01 - i32 Operations
if [ -f "$BIN_DIR/test_runner" ]; then
    run_test_suite "$BIN_DIR/test_runner" "i32 Operations & Control Flow" "01"
else
    echo -e "${RED}✗ Test runner for suite 01 not found${NC}"
    echo ""
fi

# Test Suite 02 - Floats and Function Calls
if [ -f "$BIN_DIR/test_runner_02" ]; then
    run_test_suite "$BIN_DIR/test_runner_02" "Floats, Recursion & Type Conversions" "02"
else
    echo -e "${RED}✗ Test runner for suite 02 not found${NC}"
    echo ""
fi

# Test Suite 03 - i64, Data Segments, and call_indirect
if [ -f "$BIN_DIR/test_runner_03" ]; then
    run_test_suite "$BIN_DIR/test_runner_03" "i64 Operations, Data Segments & Tables" "03"
else
    echo -e "${RED}✗ Test runner for suite 03 not found${NC}"
    echo ""
fi

# Generate comprehensive summary report
echo ""
echo -e "${BOLD}=========================================="
echo "Comprehensive Test Summary"
echo -e "==========================================${NC}"
echo ""
echo -e "Total Tests Executed: ${BOLD}$total_tests${NC}"
echo -e "${GREEN}Passed:              $passed_tests${NC}"
echo -e "${RED}Failed:              $failed_tests${NC}"
echo ""

# Calculate pass rate
if [ $total_tests -gt 0 ]; then
    pass_rate=$(awk "BEGIN {printf \"%.1f\", ($passed_tests/$total_tests)*100}")
    echo -e "${BOLD}Pass Rate:           ${pass_rate}%${NC}"
fi

echo ""

# Show failed tests if any
if [ $failed_tests -gt 0 ]; then
    echo -e "${RED}${BOLD}Failed Tests:${NC}"
    for test_name in "${failed_test_names[@]}"; do
        echo -e "${RED}  - ${test_name}${NC}"
    done
    echo ""
    echo -e "${YELLOW}Some tests failed. Review output above for details.${NC}"
    exit_code=1
else
    echo -e "${GREEN}${BOLD}All tests PASSED!${NC}"
    echo ""
    echo -e "${GREEN}WebAssembly interpreter is fully functional.${NC}"
    echo -e "${GREEN}Ready for NVIDIA engineering assessment.${NC}"
    exit_code=0
fi

echo ""
echo -e "${BOLD}==========================================${NC}"

exit $exit_code
