#pragma once

#include <CLI/CLI.hpp>
#include <optional>
#include <string>

struct Arguments {
    std::string repo_path;
    std::string preamble_file;
    std::string output_file;
    bool estimate{false};
    bool ignore_gitignore{false};
    bool output_json{false};
    bool debug{false};
    bool scrub_comments{false};
    bool verbose{false};
    std::string gptignore_file;
};

inline std::optional<Arguments> parse_arguments(CLI::App& app, int argc, char** argv) {
    Arguments args;

    // Add options
    app.add_option("-p,--preamble", args.preamble_file, "Path to preamble text file");
    app.add_option("-o,--output", args.output_file, "Path to output file");
    app.add_flag("-e,--estimate", args.estimate, "Estimate the number of tokens in the output");
    app.add_option("-i,--ignore", args.gptignore_file, "Path to .gptignore file");
    app.add_flag("-g,--ignore-gitignore", args.ignore_gitignore, "Ignore .gitignore file");
    app.add_flag("-j,--json", args.output_json, "Output JSON");
    app.add_flag("-d,--debug", args.debug, "Debug mode (no output to stdout)");
    app.add_flag("-s,--scrub-comments", args.scrub_comments, "Scrub comments from the output");
    app.add_flag("-v,--verbose", args.verbose, "Enable verbose logging");

    // Positional: repository path
    app.add_option("repo_path", args.repo_path, "Path to the Git repository")->required();

    // Parse
    try {
        app.parse(argc, argv);
    } catch(const CLI::ParseError &e) {
        // Return code from parse
        int ret = app.exit(e);
        if(ret != 0) {
            return std::nullopt;
        }
    }

    return args;
}
