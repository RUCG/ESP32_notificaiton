#ifndef READFILE_H
#define READFILE_H

#include <FS.h>
#include <Arduino.h>

// Function prototype
String readFile(fs::FS &fs, const char *path);

#endif // READFILE_H