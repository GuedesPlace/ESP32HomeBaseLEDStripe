#ifndef LEDController_h
#define LEDController_h
#include <ArduinoJson.h>

class LEDController
{
public:
    LEDController(int redPin, int greenPin, int bluePin);
    void setNewState(JsonObject jsonObject);
    JsonDocument getCurrentState();

private:
    void updateOuptutValues();
    bool m_isOn;
    int m_redPin;
    int m_bluePin;
    int m_greenPin;
    int m_brightness;
    int m_red;
    int m_green;
    int m_blue;
};
#endif