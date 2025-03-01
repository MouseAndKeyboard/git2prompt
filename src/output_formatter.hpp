#pragma once

#include "repo_scanner.hpp"
#include <string>
#include <vector>

struct FormatResult {
    bool ok{true};
    std::string error_msg;
    std::string data;
    long long tokens{0};
};

/**
 * @brief Format the repository contents as plain text (the original "---- file" style).
 *
 * @param files The collection of files with content
 * @param preamble_file Optional file to read as top-level preamble
 * @param scrub_comments If true, remove code comments
 * @param do_token_count If true, compute approximate token count
 * @return FormatResult
 */
FormatResult format_text(const std::vector<FileInfo>& files,
                         const std::string& preamble_file,
                         bool scrub_comments,
                         bool do_token_count);

/**
 * @brief Format the repository contents as JSON (an array of files with path + content).
 *
 * @param files The collection of files with content
 * @param preamble_file Optional file to read as top-level preamble
 * @param scrub_comments If true, remove code comments
 * @param do_token_count If true, compute approximate token count
 * @return FormatResult
 */
FormatResult format_json(const std::vector<FileInfo>& files,
                         const std::string& preamble_file,
                         bool scrub_comments,
                         bool do_token_count);
