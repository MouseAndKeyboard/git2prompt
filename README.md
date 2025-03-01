# git2prompt

This project is a **C++23** re-implementation of the [git2gpt](https://github.com/chand1012/git2gpt) tool, which converts a Git repository into text (or JSON) suitable for ChatGPT or other LLM prompts. It supports `.gptignore` to filter out unwanted files, optionally merges `.gitignore` rules, scrubs comments to save tokens, and can estimate the approximate token count.

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
