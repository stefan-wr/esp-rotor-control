#include <Arduino.h>
#include <LittleFS.h>
#include <SimpleFS.h>

// => Initialise FS
bool mountFS() {
  if (!LittleFS.begin()) {
      Serial.println("[FS] Mounting LittleFS failed.");
      return false;
  } else {
      return true;
  }
}

// => Read file from FS, returns the content of the file
String readFromFS(const char* path) {
  Serial.print("[FS] Reading file " + (String)path + "...");

  File file = LittleFS.open(path, FILE_READ);
  if (!file || file.isDirectory()) {
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

// => Write a String to file from FS
bool writeToFS(const char* path, const String &data) {
  Serial.print("[FS] Writing to file " + (String)path + "...");
  File file = LittleFS.open(path, FILE_WRITE);

  if (!file || file.isDirectory()) {
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

// => Remove file from FS
bool removeFromFS(const char* path) {
  if (LittleFS.remove(path)) {
    Serial.print("[FS] Deleted file ");
    Serial.println(path);
    return true;
  } else {
    Serial.print("[FS] Failed deleting file ");
    Serial.println(path);
    return false;
  }
}
