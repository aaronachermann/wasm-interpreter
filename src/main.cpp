#include "decoder.h"
#include "interpreter.h"
#include "types.h"
#include <iostream>
#include <string>
#include <vector>
#include <exception>

namespace {

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " <wasm_file> [function_name] [args...]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  <wasm_file>      Path to the WebAssembly binary file\n";
    std::cout << "  [function_name]  Name of the exported function to call (optional)\n";
    std::cout << "  [args...]        Arguments to pass to the function (optional)\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << program_name << " module.wasm\n";
    std::cout << "  " << program_name << " module.wasm add 5 10\n";
    std::cout << "\nIf no function name is provided, the module will be instantiated\n";
    std::cout << "and the start function will be executed if present.\n";
}

void printResults(const std::vector<wasm::TypedValue>& results) {
    if (results.empty()) {
        std::cout << "Function returned no values\n";
        return;
    }

    std::cout << "Results:\n";
    for (size_t i = 0; i < results.size(); i++) {
        const auto& result = results[i];
        std::cout << "  [" << i << "] " << wasm::valueTypeToString(result.type) << ": ";

        switch (result.type) {
            case wasm::ValueType::I32:
                std::cout << result.value.i32;
                break;
            case wasm::ValueType::I64:
                std::cout << result.value.i64;
                break;
            case wasm::ValueType::F32:
                std::cout << result.value.f32;
                break;
            case wasm::ValueType::F64:
                std::cout << result.value.f64;
                break;
            default:
                std::cout << "unknown";
        }
        std::cout << "\n";
    }
}

wasm::TypedValue parseArgument(const std::string& arg, wasm::ValueType type) {
    switch (type) {
        case wasm::ValueType::I32:
            return wasm::TypedValue::makeI32(std::stoi(arg));
        case wasm::ValueType::I64:
            return wasm::TypedValue::makeI64(std::stoll(arg));
        case wasm::ValueType::F32:
            return wasm::TypedValue::makeF32(std::stof(arg));
        case wasm::ValueType::F64:
            return wasm::TypedValue::makeF64(std::stod(arg));
        default:
            throw std::runtime_error("Unsupported argument type");
    }
}

} // anonymous namespace

int main(int argc, char* argv[]) {
    try {
        // Parse command line arguments
        if (argc < 2) {
            printUsage(argv[0]);
            return 1;
        }

        std::string wasm_file = argv[1];

        // Check for help flag
        if (wasm_file == "-h" || wasm_file == "--help") {
            printUsage(argv[0]);
            return 0;
        }

        std::cout << "Loading WebAssembly module: " << wasm_file << "\n";

        // Decode the WASM file
        wasm::Decoder decoder;
        wasm::Module module = decoder.parse(wasm_file);

        std::cout << "Module loaded successfully\n";
        std::cout << "  Type section: " << module.types.size() << " entries\n";
        std::cout << "  Function section: " << module.functions.size() << " functions\n";
        std::cout << "  Memory section: " << module.memories.size() << " memories\n";
        std::cout << "  Global section: " << module.globals.size() << " globals\n";
        std::cout << "  Export section: " << module.exports.size() << " exports\n";

        // List exports
        if (!module.exports.empty()) {
            std::cout << "\nExported functions:\n";
            for (const auto& exp : module.exports) {
                if (exp.kind == wasm::ExternalKind::FUNCTION) {
                    std::cout << "  - " << exp.name << "\n";
                }
            }
        }

        // Instantiate the module
        std::cout << "\nInstantiating module...\n";
        wasm::Interpreter interpreter;
        interpreter.instantiate(std::move(module));
        std::cout << "Module instantiated successfully\n";

        // If a function name is provided, call it
        if (argc >= 3) {
            std::string function_name = argv[2];
            std::cout << "\nCalling function: " << function_name << "\n";

            // Parse arguments if provided
            std::vector<wasm::TypedValue> args;
            for (int i = 3; i < argc; i++) {
                // For simplicity, assume all arguments are i32
                // In a production system, you would infer types from the function signature
                args.push_back(wasm::TypedValue::makeI32(std::stoi(argv[i])));
            }

            if (!args.empty()) {
                std::cout << "Arguments:\n";
                for (size_t i = 0; i < args.size(); i++) {
                    std::cout << "  [" << i << "] " << wasm::valueTypeToString(args[i].type)
                             << ": " << args[i].value.i32 << "\n";
                }
            }

            // Call the function
            auto results = interpreter.call(function_name, args);

            // Print results
            std::cout << "\n";
            printResults(results);
        } else {
            std::cout << "\nNo function specified. Module instantiated and start function executed (if present).\n";
            std::cout << "To call an exported function, provide its name as an argument.\n";
        }

        return 0;

    } catch (const wasm::DecoderError& e) {
        std::cerr << "Decoder error: " << e.what() << "\n";
        return 1;
    } catch (const wasm::InterpreterError& e) {
        std::cerr << "Interpreter error: " << e.what() << "\n";
        return 1;
    } catch (const wasm::Trap& e) {
        std::cerr << "WebAssembly trap: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred\n";
        return 1;
    }
}
