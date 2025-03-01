#pragma once

#include <string>
#include <vector>
#include <optional>

struct FileInfo {
    std::string relative_path;
    std::string content;
};

struct ScanResult {
    bool ok{true};
    std::string error_msg;
    std::vector<FileInfo> files;
};

/**
 * @brief Generate combined ignore patterns from .gptignore (if any) and optionally .gitignore.
 *
 * @param repo_path Path to repository
 * @param gptignore_path If non-empty, use that file. Otherwise, try repo_path/.gptignore
 * @param use_gitignore If true, also parse .gitignore patterns
 * @return vector of glob-like patterns
 */
std::vector<std::string> build_ignore_patterns(const std::string& repo_path,
                                               const std::string& gptignore_path,
                                               bool use_gitignore);

/**
 * @brief Recursively scan the repo's filesystem for files, ignoring .git and patterns in ignore list.
 * @param repo_path The root path of the repository
 * @param ignore_patterns Patterns to exclude
 * @return ScanResult
 */
ScanResult scan_repository(const std::string& repo_path, const std::vector<std::string>& ignore_patterns);
