# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

## [0.4.1] - 2026-10-16

### Changed

- refactor: Replace bitwise AND (`&=`) with logical AND (`&&`) to avoid implicit
  bool-to-int conversions and clarify boolean logic intent.

### Fixed

- Fix typos in `CHANGELOG.md`.
- Fix assert about thread-safety in `README.md`.

## [0.4.0]

### Added

- Add AVR platform support. Needs `soosp/EEPROMPreferences` for persistent
  storage and `akkoyun/MD5` for MD5 hash generation.
- Make user, password and realm buffer lengths configurable via the
  `HTTP_DIGEST_AUTH_MAX_USER_LEN`, `HTTP_DIGEST_AUTH_MAX_PASSWORD_LEN` and
  `HTTP_DIGEST_AUTH_MAX_REALM_LEN` macros.
- Mutex timeout is configurable via the `HTTP_DIGEST_AUTH_MUTEX_TIMEOUT` macro.

### Changed

- **API change:** `JSON_LEN` is now computed from the field lengths plus a
  structural overhead and reserve, instead of being configured directly.
  Removed the `HTTP_DIGEST_AUTH_MAX_JSON_LEN` macro.
- Complete the AVR platform related changes in comments and example.

## [0.3.1]

### Added

- Add basic usage example.

### Fixed

- Remove `PSTR()` macros from `snprintf()` and `Preferences` calls to avoid
  ESP8266 to crash.

## [0.3.0]

### Added

- Add ESP8266 support. ESP8266 Arduino Core contains `MD5Builder` library,
  and there is a third-party `Preferences` library can be used for persistent
  storage: `vshymanskyy/Preferences`.

### Changed

- **API change:** Renamed `MAX_JSON_LEN` macro to
  `HTTP_DIGEST_AUTH_MAX_JSON_LEN` to avoid name conflicts.
- Decreased the default length of JSON string to 256 bytes.

### Fixed

- Corrected macro configuration docs: a sketch-level `#define` has effect in
  Arduino IDE, only if the project consists of a single translation unit
  (`.ino`/`.cpp`). Documented the build-options methods and fixed the PlatformIO
  build-flag syntax (`-D NAME=VALUE`).
- Markdonwn fixes in documentation
- Use `_SIZE` constants in the examples of `README.md`.
- Fix version URLs in Changelog.
- Fix library dependency entries in `README.md`.

## [0.2.0]

### Added

- Add LICENSE file
- Add more installation instructions
- `_LEN` / `_SIZE` constants

### Changed

- `toJson` checks for truncation of JSON buffer

## [0.1.0] - 2026-05-26

### Added

- First public release

[unreleased]: https://github.com/soosp/HttpDigestAuth/compare/0.4.1...HEAD
[0.4.1]: https://github.com/soosp/HttpDigestAuth/compare/0.4.0...0.4.1
[0.4.0]: https://github.com/soosp/HttpDigestAuth/compare/0.3.1...0.4.0
[0.3.1]: https://github.com/soosp/HttpDigestAuth/compare/0.3.0...0.3.1
[0.3.0]: https://github.com/soosp/HttpDigestAuth/compare/0.2.0...0.3.0
[0.2.0]: https://github.com/soosp/HttpDigestAuth/compare/0.1.0...0.2.0
[0.1.0]: https://github.com/soosp/HttpDigestAuth/releases/tag/0.1.0
