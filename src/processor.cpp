#include "processor.hpp"
#include "repo_scanner.hpp"
#include "output_formatter.hpp"
#include "spdlog/spdlog.h"
#include <fstream>

ProcessResult process_repository(const Arguments& args) {
    ProcessResult result;

    // 1. Build ignore list
    spdlog::debug("Building ignore patterns...");
    auto ignore_patterns = build_ignore_patterns(args.repo_path, args.gptignore_file, !args.ignore_gitignore);

    // 2. Collect files from filesystem
    spdlog::debug("Scanning repository for files...");
    auto scanResult = scan_repository(args.repo_path, ignore_patterns);
    if(!scanResult.ok) {
        result.ok = false;
        result.error_msg = scanResult.error_msg;
        return result;
    }

    // 3. Format output (JSON or text), possibly scrub comments
    spdlog::debug("Formatting output...");
    auto out = (args.output_json)
               ? format_json(scanResult.files, args.preamble_file, args.scrub_comments, args.estimate)
               : format_text(scanResult.files, args.preamble_file, args.scrub_comments, args.estimate);

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
