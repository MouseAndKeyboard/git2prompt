#include "token_count.hpp"
#include <sstream>
#include <string>

long long approximate_token_count(const std::string& text) {
    // Quick approach: count whitespace-delimited words
    // Each word is approx. 1 token (rough approximation).
    long long count = 0;
    std::istringstream iss(text);
    std::string word;
    while(iss >> word) {
        count++;
    }
    return count;
}
