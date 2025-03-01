#pragma once

#include <string>

/**
 * @brief Very rough token estimate by splitting on whitespace.
 *        The official tiktoken approach is more accurate, but here we replicate
 *        the general idea in C++ for demonstration.
 *
 * @param text The string to estimate
 * @return Approximate number of tokens
 */
long long approximate_token_count(const std::string& text);
