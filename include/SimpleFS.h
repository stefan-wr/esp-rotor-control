#ifndef SIMPLEFS_H
#define SIMPLEFS_H

#include <Arduino.h>

// => Initialise FS
bool mountFS();

// => Read file from FS, retuns the content of the file
String readFromFS(const char* path);

// => Write a String to file from FS
bool writeToFS(const char* path, const String &data);

// => Remove file from FS
bool removeFromFS(const char* path);

#endif // SIMPLEFS_H
