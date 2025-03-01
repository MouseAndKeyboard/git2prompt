#include "processor.hpp"
#include "repo_scanner.hpp"
#include "output_formatter.hpp"
#include "spdlog/spdlog.h"
#include <fstream>
#include <iostream>  // for std::cin, std::getline

ProcessResult process_repository(const Arguments& args) {
    ProcessResult result;

    // 1. Build ignore list
    spdlog::debug("Building ignore patterns...");
    auto ignore_patterns = build_ignore_patterns(args.repo_path,
                                                 args.gptignore_file,
                                                 !args.ignore_gitignore);

    // We'll store final files here, either from STDIN or from scan_repository
    std::vector<FileInfo> collectedFiles;

    // If we are reading file paths from STDIN, skip auto-scan
    if (args.stdin_file_list) {
        spdlog::debug("Reading file paths from STDIN...");

        std::string path;
        while (true) {
            if(!std::getline(std::cin, path)) {
                break; // EOF or error
            }
            if(path.empty()) {
                continue; // skip blank lines
            }

            // If you still want to respect ignore patterns, do so:
            bool ignored = false;
            for(const auto &pat : ignore_patterns) {
                if(matches_pattern(path, pat)) {
                    spdlog::debug("Ignored file from STDIN: {}", path);
                    ignored = true;
                    break;
                }
            }
            if(ignored) {
                continue;
            }

            // Open and read the file
            std::ifstream ifs(path, std::ios::binary);
            if(!ifs) {
                spdlog::warn("Could not open file from STDIN: {}", path);
                continue;
            }
            std::string content((std::istreambuf_iterator<char>(ifs)),
                                 std::istreambuf_iterator<char>());

            FileInfo fi;
            fi.relative_path = path;  // or just store `path` as-is
            fi.content = std::move(content);
            collectedFiles.push_back(std::move(fi));
        }
    } else {
        // 2. Collect files from filesystem normally
        spdlog::debug("Scanning repository for files...");
        auto scanResult = scan_repository(args.repo_path, ignore_patterns);
        if (!scanResult.ok) {
            result.ok = false;
            result.error_msg = scanResult.error_msg;
            return result;
        }
        collectedFiles = std::move(scanResult.files);
    }

    // 3. Format output (JSON or text), possibly scrub comments
    spdlog::debug("Formatting output...");
    auto out = (args.output_json)
        ? format_json(collectedFiles, args.preamble_file, args.scrub_comments, args.estimate)
        : format_text(collectedFiles, args.preamble_file, args.scrub_comments, args.estimate);

    if(!out.ok) {
        result.ok = false;
        result.error_msg = out.error_msg;
        return result;
    }

    result.ok = true;
    result.output = out.data;
    result.token_estimate = out.tokens;
    return result;
}
