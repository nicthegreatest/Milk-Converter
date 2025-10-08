# Changelog

All notable changes to this project will be documented in this file.

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