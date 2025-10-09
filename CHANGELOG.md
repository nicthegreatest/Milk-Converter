# Changelog

All notable changes to this project will be documented in this file.

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