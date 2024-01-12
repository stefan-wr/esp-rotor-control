#include <Arduino.h>
#include <SPIFFS.h>
#include <SimpleSPIFFS.h>

// => Initialise SPIFFS
bool mountSPIFFS() {
  if (!SPIFFS.begin(true)) {
      Serial.println("Mounting SPIFFS failed.");
      return false;
  } else {
      return true;
  }
}

// => Read file from SPIFFS, retuns the content of the file
String readFromSPIFFS(const char* path) {
  Serial.print("Reading file " + (String)path + "...");

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
  Serial.print("Writing to file " + (String)path + "...");
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
    Serial.print("Deleted file ");
    Serial.println(path);
    return true;
  } else {
    Serial.print("Failed deleting file ");
    Serial.println(path);
    return false;
  }
}
