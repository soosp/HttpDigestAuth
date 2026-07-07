# HttpDigestAuth

Thread-safe helper class for HTTP Digest Authentication on ESP32, ESP8266
and AVR Arduino platforms.

Stores the username, realm, password and the corresponding MD5 hash in the
format required by HTTP Digest Auth: `md5("user:realm:password")`.

## Features

- Thread-safe setter and getter methods (FreeRTOS mutex on ESP32)
- Persistent storage via the `Preferences` or `EEPROMPreferences` library
- Configurable storage namespace (supports multiple instances)
- Configurable field buffer sizes
- JSON serialization of credential data

## Platform support

Each platform relies on its own MD5 and persistent-storage backend. The
third-party libraries below are pulled in automatically by PlatformIO (see
Installation); for the Arduino IDE they must be installed manually.

|Platform|Status|MD5|Persistent storage|Thread safety|
|---|---|---|---|---|
|ESP32|✅|`MD5Builder` (core)|`Preferences` (core)|FreeRTOS mutex|
|ESP8266|✅|`MD5Builder` (core)|[`vshymanskyy/Preferences`](https://github.com/vshymanskyy/Preferences)|single-threaded, no mutex|
|AVR|✅|[`akkoyun/MD5`](https://github.com/akkoyun/MD5)|[`soosp/EEPROMPreferences`](https://github.com/soosp/EEPROMPreferences)|single-threaded, no mutex|
|Other|🔧 TODO|—|—|—|

## Installation

### PlatformIO

Add the library to `platformio.ini`. The platform-specific dependencies
(MD5 and storage backends) are resolved automatically from `library.json`:

```ini
lib_deps =
    soosp/HttpDigestAuth
```

### Arduino IDE

Install **HttpDigestAuth** via the Library Manager (Sketch → Include Library
→ Manage Libraries), or download this repository as a ZIP and use
**Sketch → Include Library → Add .ZIP Library**.

The Arduino IDE does not resolve platform-specific dependencies automatically,
so install the third-party libraries for your target platform manually (see
the Platform support table above). Libraries not available in the Library
Manager can be added from their GitHub ZIP.

## Usage

### Default credentials

```cpp
#include "HttpDigestAuth.h"

// Defaults: user="admin", realm="local", password="admin"
HttpDigestAuth auth;
```

### Custom credentials

```cpp
HttpDigestAuth auth("myuser", "myrealm", "mypassword");
```

### Setting credentials

```cpp
auth.setCredentials("user", "realm", "password");
auth.setUserAndPassword("user", "password");
auth.setPassword("newpassword");
```

### Getting credentials

```cpp
char user[HttpDigestAuth::MAX_USER_SIZE];
auth.getUser(user, sizeof(user));

char hash[HttpDigestAuth::MD5_HASH_SIZE];
auth.getHash(hash, sizeof(hash));

char realm[HttpDigestAuth::MAX_REALM_SIZE];
auth.getRealm(realm, sizeof(realm));

HttpDigestAuth::Credentials creds;
auth.getCredentials(creds);  // fills user, realm, md5 (not pw)
```

### Credentials struct

```cpp
// Set all fields at once
HttpDigestAuth::Credentials creds;
snprintf(creds.user,  sizeof(creds.user),  "user");
snprintf(creds.realm, sizeof(creds.realm), "realm");
snprintf(creds.pw,    sizeof(creds.pw),    "password");
auth.setCredentials(creds);
```

### Persistent storage

```cpp
// Save to flash (default namespace: "auth")
auth.save();

// Restore from flash
auth.restore();

// Custom namespace (useful for multiple instances)
auth.save("auth2");
auth.restore("auth2");

// Clear saved data
auth.clear();
```

### JSON output

```cpp
char json[HttpDigestAuth::JSON_SIZE];
auth.toJson(json, sizeof(json));
// {"user":"admin","realm":"local","md5":"44880d12b5ed7d15aa3c2ffbb413b15c"}
```

## Multiple instances

Each instance can use a separate storage namespace to avoid conflicts:

```cpp
HttpDigestAuth admin("admin", "local", "admin");
HttpDigestAuth user("user",   "local", "pass");

admin.save("auth_admin");
user.save("auth_user");
```

## Constants

|Constant|Default|Description|
|---|---|---|
|`MAX_USER_LEN`|16|Maximum username length|
|`MAX_USER_SIZE`|17|Buffer size for username including trailing zero|
|`MAX_PASSWORD_LEN`|64|Maximum password length|
|`MAX_PASSWORD_SIZE`|65|Buffer size for password including trailing zero|
|`MAX_REALM_LEN`|64|Maximum realm length|
|`MAX_REALM_SIZE`|65|Buffer size for realm including trailing zero|
|`MD5_HASH_LEN`|32|MD5 hash length (hex string)|
|`MD5_HASH_SIZE`|33|Buffer size for MD5 hash including trailing zero|
|`JSON_LEN`|159|JSON string length, computed from the field lengths|
|`JSON_SIZE`|160|Buffer size for the JSON string including trailing zero|
|`MUTEX_TIMEOUT`|1000|Mutex timeout in milliseconds|

## Configuration

The mutex timeout, user, password and realm buffer lengths are overridable
via preprocessor macros:

|Macro|Default|
|---|---|
|`HTTP_DIGEST_AUTH_MUTEX_TIMEOUT`|1000|
|`HTTP_DIGEST_AUTH_MAX_USER_LEN`|16|
|`HTTP_DIGEST_AUTH_MAX_PASSWORD_LEN`|64|
|`HTTP_DIGEST_AUTH_MAX_REALM_LEN`|64|

`HTTP_DIGEST_AUTH_MUTEX_TIMEOUT` only takes effect on ESP32; ESP8266 and AVR
are single-threaded, so their lock is a no-op and the timeout is ignored.

`JSON_LEN` is computed automatically from these lengths, so it does not need
to be configured separately.

### Where to define these

The macros set **fixed buffer sizes**, so **every translation unit that
includes a library header must see the same value**. In a single-file sketch,
define them before the first `#include`. In a multi-file project they must be
defined *globally* — otherwise the differing buffer sizes across `.cpp` files
cause an ODR violation (undefined behaviour).

- **PlatformIO** (all platforms) — in `platformio.ini`:

  ```ini
  build_flags =
      -D HTTP_DIGEST_AUTH_MAX_USER_LEN=32
      -D HTTP_DIGEST_AUTH_MAX_PASSWORD_LEN=128
  ```

- **Arduino IDE, ESP32** — add a file named `build_opt.h` in the sketch folder.
  The core passes it to the compiler as a response file for **every**
  translation unit, so it holds **compiler flags**, not `#define`s (despite the
  `.h` name):

  ```txt
  -DHTTP_DIGEST_AUTH_MAX_USER_LEN=32
  ```

> Defining a macro before the include in only your `.ino` affects **that file
> only** — other `.cpp` files use the defaults. That mismatch is the multi-TU
> pitfall the global methods above avoid.

### Memory considerations

The field lengths determine fixed buffer sizes. Beyond the four `char` members
of each instance, the largest transient allocation is a stack buffer in the
internal MD5 helper, sized roughly `MAX_USER_LEN + MAX_REALM_LEN +
MAX_PASSWORD_LEN`. Increasing these macros raises both the per-instance
footprint and that temporary stack usage. On AVR, where RAM is scarce
(e.g. 2 KB on an ATmega328), keep the lengths conservative to avoid stack
overflow.

## Character encoding

The credential fields are recommended to be **ASCII**. Two reasons:

- **Length limits are in bytes, not characters.** A UTF-8 accented character
  takes 2–4 bytes, so `MAX_USER_LEN = 16` may hold fewer than 16 such
  characters. Truncation happens at the byte level and can cut a multi-byte
  character in half, leaving an invalid UTF-8 sequence in the buffer.
- **Hash agreement across client and server is not guaranteed.** MD5 hashes
  the raw bytes, so the result depends on the exact encoding and Unicode
  normalization (e.g. NFC vs NFD) of the input. A client that normalizes or
  encodes differently produces a different hash for the "same" password, and
  authentication fails. RFC 7616 added a `charset="UTF-8"` parameter to address
  this, but client support varies.

For interoperable Digest Authentication, keep usernames, realms and passwords
within ASCII.

## License

MIT License — Copyright (c) 2026 Péter Soós
