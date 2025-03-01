#include "repo_scanner.hpp"
#include "spdlog/spdlog.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

/**
 * @brief Simple glob-like matching. For more advanced logic, integrate a real glob library.
 *        This function attempts to match a pattern like "*.log", "dir/**", etc. using naive checks.
 */
static bool matches_pattern(const std::string& text, const std::string& pattern);

static std::vector<std::string> parse_ignore_file(const fs::path& path) {
    std::vector<std::string> patterns;
    if(!fs::exists(path)) return patterns;

    std::ifstream ifs(path);
    if(!ifs) return patterns;

    std::string line;
    while(std::getline(ifs, line)) {
        // Trim
        while(!line.empty() && std::isspace((unsigned char)line.back())) {
            line.pop_back();
        }
        if(line.empty() || line[0] == '#') {
            continue;
        }
        // If line ends with '/', add "**" to match directories fully
        if(line.back() == '/') {
            line += "**";
        }
        patterns.push_back(line);
    }
    return patterns;
}

std::vector<std::string> build_ignore_patterns(const std::string& repo_path,
                                               const std::string& gptignore_path,
                                               bool use_gitignore)
{
    std::vector<std::string> result;
    fs::path repo(repo_path);

    // Always ignore .git folder
    result.push_back(".git/**");
    result.push_back(".gitignore");
    result.push_back(".gptignore");

    // If gptignore file is specified or exists
    fs::path customGpt = gptignore_path.empty() ? (repo / ".gptignore") : fs::path(gptignore_path);
    if(fs::exists(customGpt)) {
        // we consider .gptignore to override .gitignore usage
        auto v = parse_ignore_file(customGpt);
        result.insert(result.end(), v.begin(), v.end());
    } else {
        // If user didn't provide .gptignore, we optionally parse .gitignore
        if(use_gitignore) {
            fs::path gi = repo / ".gitignore";
            if(fs::exists(gi)) {
                auto v = parse_ignore_file(gi);
                result.insert(result.end(), v.begin(), v.end());
            }
        }
    }

    // Remove duplicates
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    return result;
}

ScanResult scan_repository(const std::string& repo_path, const std::vector<std::string>& ignore_patterns) {
    ScanResult sr;
    sr.ok = true;
    fs::path base(repo_path);

    if(!fs::exists(base)) {
        sr.ok = false;
        sr.error_msg = "Repository path does not exist: " + repo_path;
        return sr;
    }
    if(!fs::is_directory(base)) {
        sr.ok = false;
        sr.error_msg = "Path is not a directory: " + repo_path;
        return sr;
    }

    // Gather all files in the repo
    try {
        for(auto const& entry : fs::recursive_directory_iterator(base)) {
            if(entry.is_regular_file()) {
                // Compute relative path
                auto rel = fs::relative(entry.path(), base).generic_string();

                // Check ignore patterns
                bool ignored = false;
                for(const auto& pat : ignore_patterns) {
                    if(matches_pattern(rel, pat)) {
                        ignored = true;
                        break;
                    }
                }
                if(ignored) {
                    spdlog::debug("Ignored file: {}", rel);
                    continue;
                }

                // Read file content
                std::ifstream ifs(entry.path(), std::ios::binary);
                if(!ifs) {
                    spdlog::warn("Could not open file: {}", rel);
                    continue;
                }
                std::string content((std::istreambuf_iterator<char>(ifs)),
                                     std::istreambuf_iterator<char>());

                // Optionally skip non-UTF8 or handle them differently
                // For simplicity, we just store as-is.
                FileInfo fi;
                fi.relative_path = rel;
                fi.content = std::move(content);

                sr.files.push_back(std::move(fi));
            } else if(entry.is_directory()) {
                // No special logic needed, we do not store directories
            }
        }
    } catch(const std::exception& e) {
        sr.ok = false;
        sr.error_msg = e.what();
        return sr;
    }

    return sr;
}

// -----------------------------------------------------------------------------
// Basic pattern matching for demonstration. For advanced usage, a third-party
// library (e.g., gh::glob) or a direct .gitignore parser could be used.
// -----------------------------------------------------------------------------
#include <regex>

static bool ends_with(const std::string& str, const std::string& suffix) {
    if(suffix.size() > str.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

static bool matches_pattern(const std::string& text, const std::string& pattern) {
    // Very naive approach:
    // 1) Replace "**" with a regex ".*"
    // 2) Replace "*" with a regex "[^/]*"
    // 3) Then do a full regex match
    // Convert pattern to a safe string for regex
    std::string re;
    re.reserve(pattern.size()*2);
    for(size_t i=0; i<pattern.size(); i++) {
        if(pattern.compare(i,2,"**") == 0) {
            re += ".*";
            i++;
        }
        else if(pattern[i] == '*') {
            re += "[^/]*";
        }
        else {
            // escape regex metachars
            if(std::string(".^$+?()[]{}|\\").find(pattern[i]) != std::string::npos) {
                re += '\\';
            }
            re.push_back(pattern[i]);
        }
    }

    // Make sure we match entire string
    re = "^" + re + "$";

    try {
        std::regex rgx(re, std::regex::ECMAScript | std::regex::icase);
        return std::regex_match(text, rgx);
    } catch(...) {
        // If invalid pattern, fallback to simple substring
        if(text.find(pattern) != std::string::npos) {
            return true;
        }
        return false;
    }
}
