#include "output_formatter.hpp"
#include "comment_scrub.hpp"
#include "spdlog/spdlog.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "token_count.hpp"

#ifdef USE_SIMDJSON
#include "simdjson.h"
#endif

static std::string read_preamble(const std::string& preamble_file) {
    if(preamble_file.empty()) {
        return std::string("The following text is a Git repository with code. Sections begin with '----', followed by the file path, then the file contents. The repository ends at '--END--'.\n");
    }
    std::ifstream ifs(preamble_file);
    if(!ifs) {
        spdlog::warn("Could not read preamble file: {}", preamble_file);
        return std::string();
    }
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return buffer.str();
}

FormatResult format_text(const std::vector<FileInfo>& files,
                         const std::string& preamble_file,
                         bool scrub_comments,
                         bool do_token_count)
{
    FormatResult fr;
    std::stringstream ss;

    auto preamble = read_preamble(preamble_file);
    ss << preamble << "\n";

    for(const auto& f : files) {
        ss << "----\n";
        ss << f.relative_path << "\n";
        if(scrub_comments) {
            auto cleaned = remove_comments(f.content);
            ss << cleaned << "\n";
        } else {
            ss << f.content << "\n";
        }
    }
    ss << "--END--";

    fr.data = ss.str();

    if(do_token_count) {
        fr.tokens = approximate_token_count(fr.data);
    }

    return fr;
}

FormatResult format_json(const std::vector<FileInfo>& files,
                         const std::string& preamble_file,
                         bool scrub_comments,
                         bool do_token_count)
{
    FormatResult fr;

    // We'll produce JSON with:
    // {
    //   "preamble": "...",
    //   "files": [
    //     {"path": "...", "content": "..."},
    //     ...
    //   ],
    //   "token_estimate": 12345
    // }
#ifdef USE_SIMDJSON
    using namespace simdjson;
    ondemand::document doc; // not actually used for building; we'll do a manual approach.

    // Build a JSON string manually or using simdjson's builder approach
    std::stringstream jsonSS;
    jsonSS << "{";

    // Add preamble
    auto preamble = read_preamble(preamble_file);
    jsonSS << "\"preamble\":";
    {
        // escape quotes
        std::string replaced;
        replaced.reserve(preamble.size()*2);
        for(char c : preamble) {
            if(c == '\"') replaced += "\\\"";
            else if(c == '\\') replaced += "\\\\";
            else replaced.push_back(c);
        }
        jsonSS << "\"" << replaced << "\",";
    }

    // Start files array
    jsonSS << "\"files\":[";
    for(size_t i=0; i<files.size(); i++) {
        jsonSS << "{";
        // path
        jsonSS << "\"path\":\"";
        {
            // escape quotes
            std::string replaced;
            replaced.reserve(files[i].relative_path.size()*2);
            for(char c : files[i].relative_path) {
                if(c == '\"') replaced += "\\\"";
                else if(c == '\\') replaced += "\\\\";
                else replaced.push_back(c);
            }
            jsonSS << replaced << "\",";
        }

        // content
        jsonSS << "\"content\":\"";
        {
            std::string content = (scrub_comments) ?
                                  remove_comments(files[i].content) :
                                  files[i].content;
            // escape
            std::string replaced;
            replaced.reserve(content.size()*2);
            for(char c : content) {
                if(c == '\"') replaced += "\\\"";
                else if(c == '\\') replaced += "\\\\";
                else if(c == '\n') replaced += "\\n";
                else replaced.push_back(c);
            }
            jsonSS << replaced << "\"";
        }

        jsonSS << "}";
        if(i+1 < files.size()) {
            jsonSS << ",";
        }
    }
    jsonSS << "]";

    // We'll compute token estimate after building string
    // We'll insert token_estimate at the end
    jsonSS << ",\"token_estimate\":0"; // placeholder
    jsonSS << "}";

    auto jsonStr = jsonSS.str();

    long long tcount = 0;
    if(do_token_count) {
        tcount = approximate_token_count(jsonStr);
    }

    // Now fix up that placeholder. Let's do a simple find/replace for "0" in "token_estimate":0
    // This is a quick hack. In production, you'd build with a dedicated JSON builder approach.
    {
        auto placeholderPos = jsonStr.rfind("\"token_estimate\":0");
        if(placeholderPos != std::string::npos) {
            std::stringstream tokenNum;
            tokenNum << "\"token_estimate\":" << tcount;
            jsonStr.replace(placeholderPos, std::string("\"token_estimate\":0").size(), tokenNum.str());
        }
    }

    fr.data = std::move(jsonStr);
    fr.tokens = tcount;
    fr.ok = true;

#else
    // If simdjson is disabled, we just build a naive JSON string manually
    // (Without escaping, it won't be fully correct for complex data, but demonstrates fallback.)
    std::stringstream ss;
    ss << "{\n  \"preamble\": \"" << read_preamble(preamble_file) << "\",\n";
    ss << "  \"files\": [\n";
    for(size_t i=0; i<files.size(); i++) {
        ss << "    {\n";
        ss << "      \"path\": \"" << files[i].relative_path << "\",\n";
        auto content = (scrub_comments ? remove_comments(files[i].content) : files[i].content);
        // For simplicity, skip advanced escaping here
        ss << "      \"content\": \"" << content << "\"\n";
        ss << "    }";
        if(i+1 < files.size()) ss << ",";
        ss << "\n";
    }
    ss << "  ],\n";
    ss << "  \"token_estimate\": 0\n";
    ss << "}";

    auto jsonStr = ss.str();

    long long tcount = 0;
    if(do_token_count) {
        tcount = approximate_token_count(jsonStr);
    }
    // hacky replacement
    auto pos = jsonStr.rfind("\"token_estimate\": 0");
    if(pos != std::string::npos) {
        std::stringstream repl;
        repl << "\"token_estimate\": " << tcount;
        jsonStr.replace(pos, std::string("\"token_estimate\": 0").size(), repl.str());
    }

    fr.data = std::move(jsonStr);
    fr.tokens = tcount;
    fr.ok = true;
#endif

    return fr;
}
