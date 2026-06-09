#pragma once
#ifdef ARDUINO_ARCH_ESP32
#include "Arduino.h"
#include <MD5Builder.h>
#include <Preferences.h>
#else
// TODO: provide platform-specific MD5, mutex and configuration saver
//       implementation
#endif

/**
 * @brief Thread-safe helper class for HTTP Digest Authentication.
 *
 * Stores the username, realm, password and the corresponding MD5 hash
 * in the format required by HTTP Digest Auth: md5("user:realm:password")
 *
 * Example:
 *   user: "admin", realm: "local", password: "admin"
 *   hash: md5("admin:local:admin") = "44880d12b5ed7d15aa3c2ffbb413b15c"
 *
 * All setter methods are protected by a mutex and safe to call from multiple tasks.
 * Getter methods return a copy into a caller-provided buffer to avoid data races.
 *
 * Platform support:
 *   - ESP32 (FreeRTOS): uses SemaphoreHandle_t
 *   - Other platforms:  uses std::timed_mutex
 */
class HttpDigestAuth {
public:
#ifndef MAX_JSON_LEN
  static constexpr size_t   JSON_LEN              = 1024;
#else
  static constexpr size_t   JSON_LEN              = MAX_JSON_LEN;
#endif
  static constexpr size_t   JSON_SIZE             =   JSON_LEN + 1;

  // Maximum lengths for authentication fields (without null terminator)
  static constexpr size_t   MAX_USER_LEN          =   16;
  static constexpr size_t   MAX_USER_SIZE         =   MAX_USER_LEN + 1;
  static constexpr size_t   MAX_PASSWORD_LEN      =   64;
  static constexpr size_t   MAX_PASSWORD_SIZE     =   MAX_PASSWORD_LEN + 1;
  static constexpr size_t   MAX_REALM_LEN         =   64;
  static constexpr size_t   MAX_REALM_SIZE        =   MAX_REALM_LEN + 1;
  static constexpr size_t   MD5_HASH_LEN          =   32;
  static constexpr size_t   MD5_HASH_SIZE         =   MD5_HASH_LEN + 1;
  // Default timeout in miliseconds for mutex 
  static constexpr uint32_t DEFAULT_MUTEX_TIMEOUT = 1000;

  /**
   * @brief Default constructor. Initializes with admin/local/admin credentials.
   */
  explicit HttpDigestAuth() : HttpDigestAuth("admin", "local", "admin") {}

  /**
   * @brief Constructor with custom credentials.
   * @param user   Username
   * @param realm  Authentication realm
   * @param pw     Password
   */
  explicit HttpDigestAuth(const char* user, const char* realm, const char* pw) {
#ifdef ARDUINO_ARCH_ESP32
    _mutex = xSemaphoreCreateMutex();
#endif
    setCredentials(user, realm, pw);
  }

  /**
   * @brief Structure to use the credential data in your code
   *        as a single datatype.
   * @details See setCredentials and getCredentials for further usage
   *          information.
   */
  struct Credentials {
    char user[MAX_USER_SIZE];
    char realm[MAX_REALM_SIZE];
    char pw[MAX_PASSWORD_SIZE];
    char md5[MD5_HASH_SIZE];
  };

  /** @brief Sets the password and updates the MD5 hash. */
  bool setPassword(const char* pw) {
    if (!_lock()) return false;
    snprintf(_pw, MAX_PASSWORD_SIZE, "%s", pw);
    _md5Hash(_md5, _user, _realm, _pw);
    _unlock();
    return true;
  }

  /** @brief Sets the username and password, then updates the MD5 hash. */
  bool setUserAndPassword(const char* user, const char* pw) {
    if (!_lock()) return false;
    snprintf(_user, MAX_USER_SIZE, "%s", user);
    snprintf(_pw, MAX_PASSWORD_SIZE, "%s", pw);
    _md5Hash(_md5, _user, _realm, _pw);
    _unlock();
    return true;
  }

  /**
   * @brief Sets all credential data and updates the MD5 hash.
   * @param user   Username
   * @param realm  Authentication realm
   * @param pw     Password
   * @return true if the mutex was acquired, false on timeout
   */
  bool setCredentials(const char* user, const char* realm, const char* pw) {
    if (!_lock()) return false;
    snprintf(_user, MAX_USER_SIZE, "%s", user);
    snprintf(_realm, MAX_REALM_SIZE, "%s", realm);
    snprintf(_pw, MAX_PASSWORD_SIZE, "%s", pw);
    _md5Hash(_md5, _user, _realm, _pw);
    _unlock();
    return true;
  }

  /**
   * @brief Sets all credential data and updates the MD5 hash.
   * @param cred Credentials structure
   * @return true if the mutex was acquired, false on timeout
   * @details It does neither use md5 field as input nor updates it.
   *          The internal MD5 hash is calculated from the other credentials.
   */
  bool setCredentials(const Credentials cred) {
    if (!_lock()) return false;
    snprintf(_user, MAX_USER_SIZE, "%s", cred.user);
    snprintf(_realm, MAX_REALM_SIZE, "%s", cred.realm);
    snprintf(_pw, MAX_PASSWORD_SIZE, "%s", cred.pw);
    _md5Hash(_md5, _user, _realm, _pw);
    _unlock();
    return true;
  }


  /**
   * @brief Copies the username into the caller-provided buffer.
   * @param buf  Destination buffer
   * @param len  Size of destination buffer
   * @return true if the mutex was acquired, false on timeout
   */
  bool getUser(char* buf, size_t len) const {
    if (!_lock()) return false;
    snprintf(buf, len, "%s", _user);
    _unlock();
    return true;
  }

  /**
   * @brief Copies the MD5 hash into the caller-provided buffer.
   *        The hash is in the format: md5("user:realm:password")
   * @param buf  Destination buffer (should be at least MD5_HASH_SIZE bytes)
   * @param len  Size of destination buffer
   * @return true if the mutex was acquired, false on timeout
   */
  bool getHash(char* buf, size_t len) const {
    if (!_lock()) return false;
    snprintf(buf, len, "%s", _md5);
    _unlock();
    return true;
  }

  /**
   * @brief Copies the Realm into the caller-provided buffer.
   * @param buf  Destination buffer (should be at least MAX_REALM_SIZE bytes)
   * @param len  Size of destination buffer
   * @return true if the mutex was acquired, false on timeout
   */
  bool getRealm(char* buf, size_t len) const {
    if (!_lock()) return false;
    snprintf(buf, len, "%s", _realm);
    _unlock();
    return true;
  }

  /**
   * @brief Gets authentication fields.
   * @param creds  Credentials structure
   * @return true if the mutex was acquired, false on timeout
   * @details It does not get pw (password) field. It used as input
   *          in setCredentials only.
   */
  bool getCredentials(Credentials& creds) const {
    if (!_lock()) return false;
    snprintf(creds.user,  MAX_USER_SIZE,  "%s", _user);
    snprintf(creds.realm, MAX_REALM_SIZE, "%s", _realm);
    snprintf(creds.md5,   MD5_HASH_SIZE,  "%s", _md5);
    _unlock();
    return true;
  }

  /**
   * @brief Converts auth data (user, realm and md5 hash) to JSON text.
   * @param json Destination buffer for JSON text.
   * @param len  Size of destination buffer
   * @return true if the mutex was acquired,
   *         false on timeout, or output truncated
   */
  bool toJson(char *json, size_t len) const {
    constexpr uint16_t bs = std::max({sizeof(_user), sizeof(_realm), sizeof(_md5)}) + 64;
    char buffer[bs];
    bool ok = true;

    // JSON starts
    json[0] = '\0';
    ok &= _strncatJson(json, PSTR("{"), len);
    // Start using private variables
    if (!_lock()) return false;
    // Username
    snprintf(buffer, bs, PSTR("\"user\":\"%s\","), _user);
    ok &= _strncatJson(json, buffer, len);
    // Realm
    snprintf(buffer, bs, PSTR("\"realm\":\"%s\","), _realm);
    ok &= _strncatJson(json, buffer, len);
    // MD5
    snprintf(buffer, bs, PSTR("\"md5\":\"%s\""), _md5);
    // End of using private variables
    _unlock();
    ok &= _strncatJson(json, buffer, len);
    // JSON ends
    ok &= _strncatJson(json, PSTR("}"), len);
    return ok;
  }

  /**
   * @brief Saves all authentication fields and the MD5 hash to the flash.
   * @param  ns The namesapace to save.
   * @return true on success, false otherwise
   */
  bool save(const char* ns = "auth") {
    bool retval = false;
    if (!_lock()) return false;
#ifdef ARDUINO_ARCH_ESP32
    Preferences p;
    retval = p.begin(ns, _RW_MODE);
    if (retval) {
      retval &= (p.putString(PSTR("user"), _user) > 0) ;
      retval &= (p.putString(PSTR("realm"), _realm) > 0);
      retval &= (p.putString(PSTR("md5"), _md5) > 0);
      retval &= (p.putBool(PSTR("isInitialized"), true) > 0);
      p.end();
    }
#endif
    _unlock();
    return retval;
  }

  /**
   * @brief Restores all authentication fields and the MD5 hash from the flash.
   * @param  ns The namespace from which to restore.
   * @return true on success, false otherwise
   */
  bool restore(const char* ns = "auth") {
    bool retval = false;
    if (!_lock()) return false;
#ifdef ARDUINO_ARCH_ESP32
    Preferences p;
    retval = (p.begin(ns, _RO_MODE));
    if (retval) {
      // Load already saved data
      retval = p.isKey(PSTR("isInitialized"));
      if (retval) {
        p.getString(PSTR("user"), _user, MAX_USER_SIZE);
        p.getString(PSTR("realm"), _realm, MAX_REALM_SIZE);
        p.getString(PSTR("md5"), _md5, MD5_HASH_SIZE);
      }
      p.end();
    }
#endif
    _unlock();
    return retval;
  }

  /**
   * @brief Clears all saved authentication data from the flash.
   * @param  ns The namespace to clear.
   * @return true on success, false otherwise
   */  
  bool clear(const char* ns = "auth") {
    bool retval = false;
    if (!_lock()) return false;
#ifdef ARDUINO_ARCH_ESP32
    Preferences p;
    retval = (p.begin(ns, _RW_MODE));
    if (retval) {
      p.clear();
      p.end();
    }
#endif
    _unlock();
    return retval;
  }


private:
#ifdef ARDUINO_ARCH_ESP32
  mutable SemaphoreHandle_t _mutex;
#else
  mutable std::timed_mutex _mutex;
#endif

  char _user [MAX_USER_SIZE];
  char _realm[MAX_REALM_SIZE];
  char _md5  [MD5_HASH_SIZE];
  char _pw   [MAX_PASSWORD_SIZE];

  /** @brief Tries to acquire the mutex. @return true on success, false on timeout. */
  bool _lock(uint32_t timeoutMs = DEFAULT_MUTEX_TIMEOUT) const {
#ifdef ARDUINO_ARCH_ESP32
    return xSemaphoreTake(_mutex, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
#else
    return _mutex.try_lock_for(std::chrono::milliseconds(timeoutMs));
#endif
  }

  /** @brief Releases the mutex. */
  void _unlock() const {
#ifdef ARDUINO_ARCH_ESP32
    xSemaphoreGive(_mutex);
#else
    _mutex.unlock();
#endif
  }

  /**
   * @brief Computes md5("user:realm:pw") and stores the result in md5.
   *        This is the format required by HTTP Digest Authentication (RFC 7616).
   */
  void _md5Hash(char* md5, const char* user, const char* realm, const char* pw) {
    constexpr size_t l = (MAX_USER_SIZE) + (MAX_REALM_SIZE) + (MAX_PASSWORD_SIZE);
    char s[l];
    snprintf(s, l, "%s:%s:%s", user, realm, pw);
    _md5Calc(s, md5);
  }

  /** @brief Computes the MD5 hash of str and stores it as a hex string in md5. */
  void _md5Calc(const char* str, char md5[MD5_HASH_SIZE]) {
    MD5Builder builder;
    builder.begin();
    builder.add(str);
    builder.calculate();
    builder.getChars(md5);
  }

  /**
   * @brief Append @p buffer to @p json without exceeding @p jsonLen bytes.
   *
   * @param json     Destination JSON string (null-terminated).
   * @param buffer   String to append.
   * @param jsonLen  Total capacity of @p json in bytes.
   * @return  @c true  — buffer written to JSON successfully
   *          @c false — output truncated
   */
  static inline bool _strncatJson(char* json, const char* buffer, size_t jsonLen) {
      size_t remaining = (strlen(json) < jsonLen)
                        ? (jsonLen - strlen(json) - 1)
                        : 0;
      if (remaining == 0) return false;
      size_t toAppend = strlen(buffer);
      strncat(json, buffer, remaining);
      return toAppend <= remaining;  // false if truncated
  }

  /** @brief Read-write mode for Preferences library */
  const bool _RW_MODE = false;

  /** @brief Read-only mode for Preferences library */
  const bool _RO_MODE = true;
};