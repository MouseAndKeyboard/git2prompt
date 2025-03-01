#include "arg_parser.hpp"
#include "processor.hpp"
#include "spdlog/spdlog.h"
#include <iostream>

int main(int argc, char** argv) {
    CLI::App app{"git2prompt: Convert a Git repo into a ChatGPT prompt"};

    // Collect CLI arguments
    auto args = parse_arguments(app, argc, argv);
    if(!args.has_value()) {
        // parse_arguments prints error if needed
        return 1;
    }

    // If verbose, set log level to debug
    if(args->verbose) {
        spdlog::set_level(spdlog::level::debug);
    } else {
        spdlog::set_level(spdlog::level::warn);
    }

    // Process repository (scan files, read, ignore, etc.)
    auto result = process_repository(*args);
    if(!result.ok) {
        spdlog::error("Error: {}", result.error_msg);
        return 1;
    }

    // Output result
    if(!result.output.empty()) {
        if(!args->output_file.empty()) {
            // Check if output file exists
            std::ifstream test(args->output_file);
            if(test.good()) {
                spdlog::error("Output file {} already exists. Aborting.", args->output_file);
                return 1;
            }
            std::ofstream ofs(args->output_file, std::ios::out | std::ios::binary);
            if(!ofs) {
                spdlog::error("Failed to open output file {} for writing.", args->output_file);
                return 1;
            }
            ofs << result.output;
        } else {
            if(!args->debug) {
                // Print to stdout if not in debug mode
                std::cout << result.output << std::endl;
            }
        }
    }

    // Print token estimate if requested
    if(args->estimate) {
        std::cout << "Estimated tokens: " << result.token_estimate << std::endl;
    }

    return 0;
}
