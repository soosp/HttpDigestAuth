/**
 * BasicUsage.ino - HttpDigestAuth library example
 *
 * Demonstrates basic usage of the HttpDigestAuth library:
 *   - Default and custom credentials
 *   - Saving and restoring credentials to/from persistent storage
 *   - Verifying the restored hash against the expected value
 *   - JSON output
 *   - Clearing saved data from persistent storage
 *
 * Expected serial output at 115200 baud.
 */

#include <HttpDigestAuth.h>

// Expected hash for default credentials: admin:local:admin
// Can be verified at: https://www.md5hashgenerator.com
// Input: "admin:local:admin"
const char* EXPECTED_HASH1 = "44880d12b5ed7d15aa3c2ffbb413b15c";
// Input: "user2:myrealm:password2"
const char* EXPECTED_HASH2 = "8f841b00b45e47178700c1ec48d4a14f";

HttpDigestAuth auth;

void printCredentials(const char* label) {
  char user[HttpDigestAuth::MAX_USER_SIZE];
  char realm[HttpDigestAuth::MAX_REALM_SIZE];
  char hash[HttpDigestAuth::MD5_HASH_SIZE];
  char json[HttpDigestAuth::JSON_SIZE];

  auth.getUser(user, sizeof(user));
  auth.getRealm(realm, sizeof(realm));
  auth.getHash(hash, sizeof(hash));
  auth.toJson(json, sizeof(json));

  Serial.print("\n--- "); Serial.print(label); Serial.println(" ---");
  Serial.print("User:  "); Serial.println(user);
  Serial.print("Realm: "); Serial.println(realm);
  Serial.print("Hash:  "); Serial.println(hash);
  Serial.print("JSON:  "); Serial.println(json);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("--- HttpDigestAuth basic example ---\n");

  // 1. Default credentials: admin / local / admin
  printCredentials("Default credentials");
  Serial.print("\nExpected hash: "); Serial.println(EXPECTED_HASH1);

  // 2. Save to persistent storage, then change credentials in memory
  if (!auth.save()) {
    Serial.println("\nSave to persistent storage has failed.");
  } else {
    auth.setCredentials("user2", "myrealm", "password2");
    printCredentials("After setCredentials()");
    Serial.print("\nExpected hash: "); Serial.println(EXPECTED_HASH2);
    // 3. Restore from persistent storage – should match the default credentials again
    if (!auth.restore()) {
        Serial.println("\nRestore from persistent storage has failed.");
    } else {
        printCredentials("After restore()");
        Serial.print("\nExpected hash: "); Serial.println(EXPECTED_HASH1);

        char hash[HttpDigestAuth::MD5_HASH_SIZE];
        auth.getHash(hash, sizeof(hash));
        if (strcmp(hash, EXPECTED_HASH1) == 0) {
          Serial.println("\nRestore OK: hash matches expected value.");
        } else {
          Serial.println("\nRestore FAILED: hash does not match expected value!");
        }
    }
  }

  // 4. Clean up: remove saved data from persistent storage
  if (auth.clear()) {
    Serial.println("\nPersistent storage cleared.");
  } else {
    Serial.println("\nPersistent storage clearing failed.");
  }
}

void loop() {}