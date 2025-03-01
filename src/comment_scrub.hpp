#pragma once

#include <string>

/**
 * @brief Remove single-line (#, //, --, ;, etc.) and block comments (/* ... * /, <!-- ... -->) from code.
 * @param code The original file content
 * @return The content with comments removed
 */
std::string remove_comments(const std::string& code);
