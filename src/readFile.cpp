#include "readFile.h"
#include <Arduino.h>
#include <FS.h>

String readFile(fs::FS &fs, const char *path)
{
    Serial.printf("Reading file: %s\n", path);
    File file = fs.open(path, "r");
    if (!file || file.isDirectory())
    {
        Serial.println("Failed to open file");
        return String();
    }

    String content;
    while (file.available())
    {
        content += String((char)file.read());
    }
    file.close();
    return content;
}