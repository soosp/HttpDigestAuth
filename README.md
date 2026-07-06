# HttpDigestAuth

Thread-safe helper class for HTTP Digest Authentication on ESP32, ESP8266
and AVR Arduino platforms.

Stores the username, realm, password and the corresponding MD5 hash in the
format required by HTTP Digest Auth: `md5("user:realm:password")`.

## Features

- Thread-safe setter and getter methods (FreeRTOS mutex on ESP32)
- Persistent storage via the `Preferences` or `EEPROMPreferences` library
- Configurable storage namespace (supports multiple instances)
- JSON serialization of credential data

## Requirements

### ESP32

- ESP32 Arduino Core
- `MD5Builder` and `Preferences` (both included in ESP32 Arduino Core)

### ESP8266

- ESP8266 Arduino Core
- `MD5Builder` (included in ESP8266 Arduino Core)
- [`vshymanskyy/Preferences`](https://github.com/vshymanskyy/Preferences) —
  third-party library required for `save()`/`restore()`/`clear()`

### AVR

- AVR Arduino Core
- [`akkoyun/MD5`](https://github.com/akkoyun/MD5) — third-party library
  required for MD5 hash generation
- [`soosp/EEPROMPreferences`](https://github.com/soosp/EEPROMPreferences) —
  third-party library required for `save()`/`restore()`/`clear()`

## Installation

### Arduino IDE

**Install via Library Manager:**

Search for **HttpDigestAuth** in the Library Manager (Sketch → Include Library
→ Manage Libraries).

**Install manually:**

1. Download this repository as a ZIP file
2. In Arduino IDE: **Sketch → Include Library → Add .ZIP Library**

### PlatformIO

```ini
lib_deps =
    soosp/HttpDigestAuth
```

### Manual

Copy the `HttpDigestAuth.h` file into your project or library folder.

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

## Constants

|Constant|Value|Description|
|---|---|---|
|`MAX_USER_LEN`|16|Maximum username length|
|`MAX_USER_SIZE`|17|Buffer size for username including trailing zero|
|`MAX_PASSWORD_LEN`|64|Maximum password length|
|`MAX_PASSWORD_SIZE`|65|Buffer size for password including trailing zero|
|`MAX_REALM_LEN`|64|Maximum realm length|
|`MAX_REALM_SIZE`|65|Buffer size for realm including trailing zero|
|`MD5_HASH_LEN`|32|MD5 hash length (hex string)|
|`MD5_HASH_SIZE`|33|Buffer size for MD5 hash including trailing zero|
|`JSON_LEN`|256|Default JSON buffer size (overridable via `HTTP_DIGEST_AUTH_MAX_JSON_LEN`)|
|`JSON_SIZE`|257|Size of the JSON buffer including trailing zero|
|`DEFAULT_MUTEX_TIMEOUT`|1000|Mutex timeout in milliseconds|

## Configuration

The default length of JSON buffer is overridable via the `HTTP_DIGEST_AUTH_MAX_JSON_LEN`
preprocessor macro.

### Where to define this

The macro set **fixed buffer size**, so **every translation unit that
includes a library header must see the same value**. In a single-file sketch,
define it before the first `#include`. In a multi-file project it must be
defined *globally* — otherwise the differing buffer sizes across `.cpp` files
cause an ODR violation (undefined behaviour).

- **PlatformIO** (all platforms) — in `platformio.ini`:

  ```ini
  build_flags =
      -D HTTP_DIGEST_AUTH_MAX_JSON_LEN=512
  ```

- **Arduino IDE, ESP32** — add a file named `build_opt.h` in the sketch folder.
  The core passes it to the compiler as a response file for **every**
  translation unit, so it holds **compiler flags**, not `#define`s (despite the
  `.h` name):

  ```txt
  -HTTP_DIGEST_AUTH_MAX_JSON_LEN=512
  ```

> Defining a macro before the include in only your `.ino` affects **that file
> only** — other `.cpp` files use the defaults. That mismatch is the multi-TU
> pitfall the global methods above avoid.

## Multiple instances

Each instance can use a separate storage namespace to avoid conflicts:

```cpp
HttpDigestAuth admin("admin", "local", "admin");
HttpDigestAuth user("user",   "local", "pass");

admin.save("auth_admin");
user.save("auth_user");
```

## Platform support

|Platform|Status|Notes|
|---|---|---|
|ESP32 (Arduino)|✅ Supported|Full support including persistent storage|
|ESP8266 (Arduino)|✅ Supported|No mutex needed (single-threaded); persistent storage requires [`vshymanskyy/Preferences`](https://github.com/vshymanskyy/Preferences)|
|AVR (Arduino)|✅ Supported|No mutex needed (single-threaded); MD5 hash requires [akkoyun/MD5](https://github.com/akkoyun/MD5); persistent storage requires [`soosp/EEPROMPreferences`](https://github.com/soosp/EEPROMPreferences)|
|Other platforms|🔧 TODO|Requires mutex, MD5 and storage implementation|

### ESP8266 persistent storage

Add the dependency to your `platformio.ini`:

```ini
lib_deps =
    soosp/HttpDigestAuth
    vshymanskyy/Preferences
```

For Arduino IDE, install `Preferences` via the Library Manager.

### AVR MD5 and persistent storage

Add the dependency to your `platformio.ini`:

```ini
lib_deps =
    soosp/HttpDigestAuth
    soosp/EEPROMPreferences
    akkoyun/MD5
```

For Arduino IDE, install `EEPROMPreferences` via the Library Manager and
install [`MD5`](https://github.com/akkoyun/MD5) by downloading ZIP file
and extract it to the Arduino `libraries` directory.

## License

MIT License — Copyright (c) 2026 Péter Soós
