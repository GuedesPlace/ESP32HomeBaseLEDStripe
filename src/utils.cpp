#include <Arduino.h>

String GetHostName() {
    char buffer[32];
    sprintf(buffer,"esp32-hb-%012llx", ESP.getEfuseMac());
    return String(buffer);
}