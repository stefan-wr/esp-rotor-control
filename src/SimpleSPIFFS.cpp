#include <Arduino.h>
#include <SPIFFS.h>
#include <SimpleSPIFFS.h>

// => Initialise SPIFFS
bool mountSPIFFS() {
  if (!SPIFFS.begin()) {
      Serial.println("[SPIFFS] Mounting SPIFFS failed.");
      return false;
  } else {
      return true;
  }
}

// => Read file from SPIFFS, retuns the content of the file
String readFromSPIFFS(const char* path) {
  Serial.print("[SPIFFS] Reading file " + (String)path + "...");

  File file = SPIFFS.open(path);
  if (!file) {
    Serial.println("failed to open the file.");
    return String();
  }

  String content;
  while (file.available()) {
    content += file.readString();
  }
  file.close();
  Serial.println("success.");
  return content;
}

// => Write a String to file from SPIFFS
bool writeToSPIFFS(const char* path, const String &data) {
  Serial.print("[SPIFFS] Writing to file " + (String)path + "...");
  File file = SPIFFS.open(path, FILE_WRITE);

  if (!file) {
    Serial.println("failed to open the file.");
    return false;
  }

  if (file.print(data)) {
    Serial.println("success.");
    file.close();
    return true;
  } else {
    Serial.println("writing failed.");
    file.close();
    return false;
  }
}

// => Remove file from SPIFFS
bool removeFromSPIFFS(const char* path) {
  if (SPIFFS.remove(path)) {
    Serial.print("[SPIFFS] Deleted file ");
    Serial.println(path);
    return true;
  } else {
    Serial.print("[SPIFFS] Failed deleting file ");
    Serial.println(path);
    return false;
  }
}
