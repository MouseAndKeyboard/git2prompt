# git2prompt

This project is a **C++23** re-implementation of the [git2gpt](https://github.com/chand1012/git2gpt) tool, which converts a Git repository into text (or JSON) suitable for ChatGPT or other LLM prompts. It supports `.gptignore` to filter out unwanted files, optionally merges `.gitignore` rules, scrubs comments to save tokens, and can estimate the approximate token count.

#  Download
https://github.com/MouseAndKeyboard/git2prompt/releases


## Features

- **Fast** parallel file reading and processing, using C++17/20 parallel algorithms.
- **Modern CLI** powered by [CLI11](https://github.com/CLIUtils/CLI11).
- **Ignore logic** that respects `.gitignore` or is overridden by `.gptignore`.
- **JSON output** (via [simdjson](https://github.com/simdjson/simdjson)) or **plain text** output with a custom delimiter format.
- **Comment scrubbing** (removes single- and multi-line comments) to reduce token usage.
- **Basic token estimation** by splitting text on whitespace (an approximation to ChatGPT tokens).

## Requirements

- A C++23-capable compiler (tested on GCC 13+, Clang 16+, and MSVC 2022).
- [CMake](https://cmake.org/) 3.16 or later (for FetchContent usage and modern config).
- [Git](https://git-scm.com/) installed (if you want to version-control this repo).

## Building

1. **Clone** this repository:
   ```bash
   git clone https://github.com/youruser/git2prompt.git
   cd git2prompt
   ```

2. **Configure** and **build** in **Release** mode:
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release .
   cmake --build build -j
   ```
   This will produce an optimized executable under `build/`.

3. (Optional) **Debug** build with extra checks:
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Debug .
   cmake --build build -j
   ```

4. **Run Tests**:
   ```bash
   cmake --build build --target test
   ```
   or
   ```bash
   cd build
   ctest --output-on-failure
   ```

## Usage

After building, the executable **git2prompt** is located in `build/` (or your chosen build directory). Example:

```bash
./build/git2prompt [flags] /path/to/repo
```

### Command-Line Flags

- `-p, --preamble <file>`  
  Path to a text file containing a preamble to include at the beginning of output.
- `-o, --output <file>`  
  Output file path. If omitted, prints to stdout.
- `-e, --estimate`  
  Estimate the approximate token count of the output.
- `-j, --json`  
  Output repository data in JSON format.
- `-i, --ignore <file>`  
  Path to a custom `.gptignore` file. Overrides `.gitignore` usage if provided.
- `-g, --ignore-gitignore`  
  If set, **ignore** the `.gitignore` file (do not skip files listed there).
- `-s, --scrub-comments`  
  Remove comments from files to save tokens.
- `-v, --verbose`  
  Enable verbose logging.

### Example Commands

- **Plain text output**:
  ```bash
  ./git2prompt -p preamble.txt -o repo.txt /some/repo
  ```
- **JSON output** (requires `--json`):
  ```bash
  ./git2prompt --json --output repo.json /some/repo
  ```
- **Token estimation**:
  ```bash
  ./git2prompt --estimate /some/repo
  ```

### .gptignore

- If a `.gptignore` file is present in the repo or specified via `-i`, it **overrides** reading `.gitignore` unless `-g` is used.  
- If you only want to rely on `.gptignore` (ignoring `.gitignore`), just don’t use `-g`.  
- If you want to skip `.gitignore` entirely, use `--ignore-gitignore`.

## License

See [LICENSE](LICENSE) for details.


# Description to Give to ChatGPT

```
The program, referred to as “git2prompt,” is a C++ application that collects code from a specified repository (either by scanning the entire folder or by reading file paths from standard input), optionally removes comments, and then outputs the collected files in either a plain text format with custom delimiters or in JSON form. It can also estimate the approximate number of tokens (words) contained in the output. Below is an extremely thorough description of each component and the overall flow, presented without including the raw C++ code blocks.

The program begins with a main entry point that sets up a command-line interface (CLI) using a library known as CLI11. The CLI configuration includes options such as a “preamble” file, an output file path, a flag to estimate tokens, a flag to ignore the “.gitignore” file, a flag to produce JSON output, a debug flag that suppresses regular output, a flag to remove comments from the code, a verbose mode that increases logging detail, and an optional pointer to a custom “.gptignore” file. Additionally, it provides a special flag that instructs the program to read filenames from standard input rather than automatically discovering them in the repository folder.

After gathering these arguments from the user, the program sets the logging level (using a logging library called spdlog) to debug or warn, depending on whether verbose mode was requested. It then calls a high-level routine that orchestrates all operations, named process_repository. The application then checks the result for success or failure. If the process fails (for example, due to a bad file path), it logs an error and exits. If the operation is successful, the program either prints or writes the resulting text or JSON to the desired location. The program can also print out an estimated token count if that was requested. Debug mode suppresses ordinary console output, though logs at warning or debug levels may still appear.

Within the command-line parsing mechanism, there is a specialized Arguments structure that holds fields for each possible command-line parameter, including booleans for things like whether JSON output is requested and whether to scrub comments. The repository path is stored in a string, preamble and output paths are also stored, and so on. These fields are populated by CLI11, which interprets command-line flags and arguments, assigning them to the structure.

The process_repository function is the central coordinator of the entire tool. It starts by calling a function that constructs a list of ignore patterns. These patterns might come from a “.gptignore” file if one exists, or from a “.gitignore” file if the user has not disabled its usage and no override is provided. The code also injects some default ignores for the “.git” directory or for the ignore files themselves, so that they are not included in the final output. The result is a series of glob-like rules that can be matched against file paths in order to skip them.

Once the list of ignore patterns has been built, the process_repository function decides whether to read file paths from standard input or to scan the repository folder. If the user supplied the special flag for reading from standard input, the function repeatedly reads lines until it reaches the end of input. Each line is treated as a potential file path, checked against the ignore patterns, and if it is not ignored, loaded into memory. If the file cannot be opened, the function logs a warning. The content is stored along with the path in a FileInfo structure for further processing.

On the other hand, if the user did not request reading file paths from standard input, the function calls a routine named scan_repository, which uses the C++ filesystem library to recursively walk through the provided repository path. For each regular file discovered, the path is computed relative to the repository root, and this relative path is tested against the collected ignore patterns. If it matches an ignore pattern, the file is skipped. Otherwise, the file is opened and read into a FileInfo structure. If any error occurs, such as the path not existing or not being a directory, the function returns an error.

After the files have been identified and read into memory, process_repository next invokes one of two output-formatting routines. If the user has requested JSON output, it calls format_json; otherwise, it calls format_text. Both of these routines accept a list of the collected files, a path to a preamble file (if any), a boolean indicating whether to scrub out comments, and a boolean indicating whether to generate an approximate token count.

When preparing the final text, the chosen formatting routine first decides whether to read a user-specified preamble file. If no file was indicated, a default message is used, describing how the output is structured into sections beginning with “----” and ending with “--END--.” The function then either proceeds to output data in a plain text style or in JSON. If using plain text style, each file is separated by a delimiter line containing “----.” Right after the delimiter, the file’s relative path is printed, followed by either the original file content or a version with comments removed, depending on whether comment scrubbing was requested. Finally, the output ends with “--END--” to signal that no more files follow.

For JSON output, the program creates a JSON object whose fields include a “preamble” property, a “files” array, and a “token_estimate.” Each entry in the “files” array has the fields “path” and “content,” corresponding to a file’s path and text. If comment scrubbing is requested, it is applied to each file’s content beforehand. Special characters (like quotes, backslashes, and newlines) are escaped appropriately. The JSON version can optionally use a library (simdjson) to assist with building the string and escaping; if that library is disabled, the program falls back to a simpler string-building approach.

In both plain text and JSON modes, there is an option to compute an approximate token count. This happens if the user supplied the --estimate flag. The approximate token counting is accomplished by a function named approximate_token_count, which splits the final output string on whitespace. Every chunk of text separated by whitespace is counted as a single token. This simplistic approach is not as exact as a specialized tokenizer, but it serves as a coarse estimate. If the estimate was requested, the final step of process_repository is to record that token count into the ProcessResult. If JSON output is being built, the token estimate is embedded in the resulting JSON. If plain text is used, the token estimate is simply stored in the result so that main can optionally print it at the end.

Regarding comment scrubbing, the removal of comments is handled by a dedicated remove_comments function. It tries to remove both block comments (C-style “/* ... */” or HTML-style “<!-- ... -->”) and single-line patterns (like “//”, “#”, “--”, “;”). It first uses a regex replacement to remove multi-line patterns and any double-slash plus text to the end of the line. It then processes the file line by line, discarding lines that match certain single-line comment prefixes. This method is not language-perfect, but it is good enough to strip out the bulk of commented material in many languages, which can help save space and tokens.

Overall, the program’s flow begins with command-line parsing to gather all flags and arguments. It logs at different levels depending on whether verbose mode is active. It then either loads files from the repository itself or from a list of paths provided via standard input. It applies ignore rules, scrubs comments if desired, compiles everything into either JSON or plain text, and estimates tokens if so instructed. The end result can be written to a file or printed directly to the console. Debug mode suppresses direct printing to standard output, but logging still happens. The program gracefully exits with a nonzero status in case of errors such as invalid paths or an inability to open files, and it prints the error message that explains the issue.
```

