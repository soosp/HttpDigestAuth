# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Changed

- **API change:** Renamed `MAX_JSON_LEN` macro to `HTTP_DIGEST_AUTH_MAX_JSON_LEN`
  to avoid name conflicts.
- Decreased the default length of JSON string to 256 bytes.

### Fixed

- Corrected macro configuration docs: a sketch-level `#define` has effect in
  Arduino IDE, only if the project consists of a single translation unit
  (`.ino`/`.cpp`). Documented the build-options methods and fixed the PlatformIO
  build-flag syntax (`-D NAME=VALUE`).

## [0.2.0]

### Added

- Add LICENSE file
- Add more installation instructions
- \_LEN / \_SIZE constants

### Changed

- `toJson` checks for truncation of JSON buffer

## [0.1.0] - 2026-05-26

### Added

- First public release

[unreleased]: https://github.com/soosp/RunningStatistics/compare/0.2.0...HEAD
[0.2.0]: https://github.com/soosp/RunningStatistics/releases/tag/0.1.0...0.2.0
[0.1.0]: https://github.com/soosp/RunningStatistics/releases/tag/0.1.0
