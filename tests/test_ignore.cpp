#include <gtest/gtest.h>
#include "repo_scanner.hpp"

TEST(IgnoreTest, CombinePatterns) {
    std::string repo_path = "fake_repo";
    std::string gptignore_file; // empty => not using custom file
    bool use_gitignore = true;

    auto patterns = build_ignore_patterns(repo_path, gptignore_file, use_gitignore);

    // By default, we always have .git/**, .gitignore, .gptignore
    // Then if .gptignore doesn't exist, we read .gitignore if it exists.
    // We don't have an actual file here, but let's see if the default patterns are present:
    bool hasGitPattern = false;
    for(const auto& p : patterns) {
        if(p.find(".git/") != std::string::npos) {
            hasGitPattern = true;
            break;
        }
    }
    ASSERT_TRUE(hasGitPattern);
}
