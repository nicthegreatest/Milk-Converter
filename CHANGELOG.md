# Changelog

All notable changes to this project will be documented in this file.

## [0.5.0] - 2025-10-08

### Fixed
- **Resolved a deep, silent parsing failure in the `projectm-eval` library.** This was a complex issue with multiple layers that caused the converter to produce incomplete shaders with no errors.
    - **Initial Symptom:** The converter would run successfully but produce a `.frag` file where all per-frame and per-pixel logic was missing, and many uniform values were zeroed out.
    - **Diagnostic Process:**
        1.  **Locale-related float parsing:** The first issue identified was that the C++ `std::setlocale` was not set. On systems with a comma as a decimal separator, this caused the `projectm-eval` library to parse all floating-point numbers as 0. This was fixed by setting the numeric locale to "C" at runtime.
        2.  **Build System Investigation:** The investigation then moved to the `projectM` submodule's build process. Several issues were found and fixed, including missing `libgl1-mesa-dev`, `bison`, and `flex` dependencies, and a CMake versioning problem that caused undefined version macros during compilation.
        3.  **Silent Parser Failure:** Despite a correct build environment, the parser continued to fail silently. By instrumenting the converter with extensive debugging (including printing the raw parser state machine trace), it was determined that the `projectm-eval` parser was not returning an error but was instead returning a valid, empty Abstract Syntax Tree (AST).
        4.  **Grammar Analysis:** A deep analysis of the `Compiler.y` (Bison grammar) and `Scanner.l` (Flex lexer) files revealed the final root cause.
    - **Root Cause:** The `projectm-eval` grammar contained a flawed rule for handling "empty statements" (`instruction-list: instruction-list ';' empty-expression`). This rule was causing the parser to incorrectly terminate when it encountered a list of statements, which is the format of the code from the `.milk` files.
    - **The Definitive Fix:** The faulty `empty-expression` rule was removed from the `Compiler.y` grammar. This makes the parser stricter and prevents it from misinterpreting valid, multi-statement code blocks, finally resolving the conversion failure.

### Changed
- The `projectm-eval` submodule, a dependency of `projectM`, has been patched directly to correct the faulty parser grammar.

## [0.4.0] - 2025-10-08

### Added
- Implemented a robust, AST-based translation system using the `projectm-eval` library for accurate Milkdrop to GLSL conversion.
- Finalized the build system to correctly link against the `projectM` C++ interface.

### Fixed
- Resolved a critical bug where the `per_frame` and `per_pixel` code blocks were not being correctly extracted from `.milk` files.
- Corrected a persistent `[-Wformat-extra-args]` warning in the vendored `GLSLGenerator.cpp` file.

### Known Issues
- The project is currently blocked by a persistent file system issue within the test environment's `run_in_bash_session` tool. This prevents the execution of the converter and the validation of the generated shaders. All attempts to diagnose and resolve this tool-related issue have been unsuccessful.

## [0.3.0] - 2025-10-08

### Added
- Implemented the core translation logic to convert MilkDrop `per_frame` and `per_pixel` equations to GLSL.
- Added support for generating JSON-annotated `uniform` variables for real-time UI controls in RaymarchVibe.
- Created a comprehensive mapping of MilkDrop built-in variables and functions to their GLSL equivalents.
- Added logic to detect and declare user-defined variables from preset code.

### Fixed
- Finalized the CMake build process, resolving all dependency and linker errors.
- Corrected several bugs in the expression translation, including variable substitution order and float literal conversion.

## [0.2.0] - 2025-10-08

### Added
- Created a self-contained `Converter` project directory.
- Added a detailed `README.md` file with project summary, technical plan, and build instructions.
- Added this `CHANGELOG.md` file.
- Added a `TODO.md` file to track future work.

### Changed
- Refactored the project to be a standalone command-line tool, independent of the RaymarchVibe build process.
- Vendored the `libprojectM` source code into the `Converter` directory.

## [0.1.0] - 2025-10-08

### Added
- Initial proof-of-concept `MilkdropConverter.cpp` file.
- Investigated the `.milk` file format and the `projectm` source code.
- Acquired the official MilkDrop Preset Authoring Guide.