#pragma once

#include "arg_parser.hpp"
#include <string>

struct ProcessResult {
    bool ok{false};
    std::string error_msg;
    std::string output;
    long long token_estimate{0};
};

/**
 * @brief High-level function that orchestrates reading the repo, building ignore lists,
 *        collecting files, generating output (text or JSON), and computing token estimates.
 *
 * @param args - parsed command-line arguments
 * @return ProcessResult - structure with success/failure, the final output text/JSON, and token count
 */
ProcessResult process_repository(const Arguments& args);
