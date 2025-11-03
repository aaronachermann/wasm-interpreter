#include "../include/decoder.h"
#include <iostream>
#include <vector>
#include <iomanip>

/**
 * Simple test program to verify the WebAssembly decoder.
 * Creates a minimal valid WASM module and parses it.
 */

// Create a minimal valid WASM module
// (module
//   (func $add (param $a i32) (param $b i32) (result i32)
//     local.get $a
//     local.get $b
//     i32.add)
//   (export "add" (func $add))
// )
std::vector<uint8_t> createMinimalWasmModule() {
    std::vector<uint8_t> bytes;

    // Magic number: 0x00 0x61 0x73 0x6D ("\0asm")
    bytes.push_back(0x00);
    bytes.push_back(0x61);
    bytes.push_back(0x73);
    bytes.push_back(0x6D);

    // Version: 1
    bytes.push_back(0x01);
    bytes.push_back(0x00);
    bytes.push_back(0x00);
    bytes.push_back(0x00);

    // Type section
    bytes.push_back(0x01);  // Section ID: Type
    bytes.push_back(0x07);  // Section size: 7 bytes
    bytes.push_back(0x01);  // 1 type
    bytes.push_back(0x60);  // func type
    bytes.push_back(0x02);  // 2 parameters
    bytes.push_back(0x7F);  // i32
    bytes.push_back(0x7F);  // i32
    bytes.push_back(0x01);  // 1 result
    bytes.push_back(0x7F);  // i32

    // Function section
    bytes.push_back(0x03);  // Section ID: Function
    bytes.push_back(0x02);  // Section size: 2 bytes
    bytes.push_back(0x01);  // 1 function
    bytes.push_back(0x00);  // type index 0

    // Export section
    bytes.push_back(0x07);  // Section ID: Export
    bytes.push_back(0x07);  // Section size: 7 bytes
    bytes.push_back(0x01);  // 1 export
    bytes.push_back(0x03);  // name length: 3
    bytes.push_back('a');
    bytes.push_back('d');
    bytes.push_back('d');
    bytes.push_back(0x00);  // export kind: function
    bytes.push_back(0x00);  // function index 0

    // Code section
    bytes.push_back(0x0A);  // Section ID: Code
    bytes.push_back(0x09);  // Section size: 9 bytes
    bytes.push_back(0x01);  // 1 function body
    bytes.push_back(0x07);  // body size: 7 bytes
    bytes.push_back(0x00);  // 0 local declarations
    // Function body: local.get 0, local.get 1, i32.add, end
    bytes.push_back(0x20);  // local.get
    bytes.push_back(0x00);  // local index 0
    bytes.push_back(0x20);  // local.get
    bytes.push_back(0x01);  // local index 1
    bytes.push_back(0x6A);  // i32.add
    bytes.push_back(0x0B);  // end

    return bytes;
}

int main() {
    try {
        std::cout << "WebAssembly Decoder Test\n";
        std::cout << "=========================\n\n";

        // Create minimal WASM module
        std::vector<uint8_t> bytes = createMinimalWasmModule();
        std::cout << "Created minimal WASM module (" << bytes.size() << " bytes)\n\n";

        // Print hex dump
        std::cout << "Hex dump:\n";
        for (size_t i = 0; i < bytes.size(); i++) {
            if (i > 0 && i % 16 == 0) {
                std::cout << "\n";
            }
            std::cout << std::hex << std::setfill('0') << std::setw(2)
                     << static_cast<int>(bytes[i]) << " ";
        }
        std::cout << std::dec << "\n\n";

        // Parse the module
        std::cout << "Parsing module...\n";
        wasm::Decoder decoder;
        wasm::Module module = decoder.parseBytes(bytes);

        std::cout << "Successfully parsed!\n\n";

        // Print parsed module information
        std::cout << "Module contents:\n";
        std::cout << "  Type section: " << module.types.size() << " entries\n";
        if (!module.types.empty()) {
            std::cout << "    Type 0: " << module.types[0].params.size() << " params, "
                     << module.types[0].results.size() << " results\n";
        }

        std::cout << "  Function section: " << module.function_types.size() << " functions\n";
        std::cout << "  Code section: " << module.functions.size() << " function bodies\n";

        if (!module.functions.empty()) {
            std::cout << "    Function 0: " << module.functions[0].locals.size()
                     << " locals, " << module.functions[0].body.size() << " bytes of code\n";
        }

        std::cout << "  Export section: " << module.exports.size() << " exports\n";
        for (const auto& exp : module.exports) {
            std::cout << "    Export: \"" << exp.name << "\" (kind="
                     << static_cast<int>(exp.kind) << ", index=" << exp.index << ")\n";
        }

        std::cout << "\nDecoder test PASSED!\n";
        return 0;

    } catch (const wasm::DecoderError& e) {
        std::cerr << "Decoder error: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
