#ifndef SIMPLESPIFFS_H
#define SIMPLESPIFFS_H

#include <Arduino.h>

// => Initialise SPIFFS
bool mountSPIFFS();

// => Read file from SPIFFS, retuns the content of the file
String readFromSPIFFS(const char* path);

// => Write a String to file from SPIFFS
bool writeToSPIFFS(const char* path, const String &data);

// => Remove file from SPIFFS
bool removeFromSPIFFS(const char* path);

#endif // SIMPLESPIFFS_H
