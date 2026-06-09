# HttpDigestAuth

Thread-safe helper class for HTTP Digest Authentication on ESP32.

Stores the username, realm, password and the corresponding MD5 hash in the format required by HTTP Digest Auth: `md5("user:realm:password")`.

## Features

- Thread-safe setter and getter methods (FreeRTOS mutex on ESP32)
- Persistent storage via the ESP32 `Preferences` library
- Configurable storage namespace (supports multiple instances)
- JSON serialization of credential data

## Requirements

- ESP32 Arduino Core
- `MD5Builder` and `Preferences` (both included in ESP32 Arduino Core)

## Installation

### Arduino IDE

**Install via Library Manager:**

Search for **HttpDigestAuth** in the Library Manager (Sketch → Include Library → Manage Libraries).

**Install manually:**

1. Download this repository as a ZIP file
2. In Arduino IDE: **Sketch → Include Library → Add .ZIP Library**

### PlatformIO

```ini
lib_deps =
    https://github.com/soosp/HttpDigestAuth
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
char user[HttpDigestAuth::MAX_USER_LEN + 1];
auth.getUser(user, sizeof(user));

char hash[HttpDigestAuth::MD5_HASH_LEN + 1];
auth.getHash(hash, sizeof(hash));

char realm[HttpDigestAuth::MAX_REALM_LEN + 1];
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
char json[HttpDigestAuth::JSON_LEN];
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
|`JSON_LEN`|1024|Default JSON buffer size (overridable via `MAX_JSON_LEN`)|
|`JSON_SIZE`|1025|Size of the JSON buffer including trailing zero|
|`DEFAULT_MUTEX_TIMEOUT`|1000|Mutex timeout in milliseconds|

## Multiple instances

Each instance can use a separate storage namespace to avoid conflicts:

```cpp
HttpDigestAuth admin("admin", "local", "admin");
HttpDigestAuth user("user",   "local", "pass");

admin.save("auth_admin");
user.save("auth_user");
```

## Platform support

|Platform|Status|
|---|---|
|ESP32 (Arduino)|✅ Supported|
|Other platforms|🔧 TODO (requires MD5, mutex, and storage implementation)|

## License

MIT License — Copyright (c) 2026 Péter Soós
