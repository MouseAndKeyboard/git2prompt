#include "comment_scrub.hpp"
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <istream>
#include <fstream>
#include <algorithm>

/**
 * A simple approach:
 * 1) Remove block comments: /* ... * /, <!-- ... --> (non-greedy)
 * 2) Remove single-line: //..., #..., --..., ;..., etc.
 *
 * This is not language-perfect but helps reduce tokens in code-like files.
 */
std::string remove_comments(const std::string& code) {
    // Remove multi-line comments:
    //  - C-style /* ... */
    //  - HTML <!-- ... -->
    // Use a single big regex that tries both.
    std::string result = std::regex_replace(
        code,
        std::regex(R"((?s)(/\*.*?\*/)|(<!--.*?-->)|(//[^\n]*))", 
                   std::regex::ECMAScript),
        ""
    );

    // Next, remove single-line patterns that appear at the start or mid-line:
    //   ^#.*$, ^--.*$, ^;.*$, etc.
    //   We'll do this line by line to keep newlines intact (some approximate approach).
    std::stringstream in(result);
    std::stringstream out;
    std::string line;
    static const std::regex singleLinePat(R"(^\s*(#|--|;|%)+(.*))");

    while(std::getline(in, line)) {
        // check if line matches
        if(std::regex_match(line, singleLinePat)) {
            // remove line entirely
            continue;
        }
        out << line << "\n";
    }

    return out.str();
}
