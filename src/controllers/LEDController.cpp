#include "LEDController.h"
#include <ArduinoJson.h>
#include <Arduino.h>
LEDController::LEDController(int redPin, int greenPin, int bluePin)
{
    m_redPin = redPin;
    m_greenPin = greenPin;
    m_bluePin = bluePin;
    m_isOn = false;
    m_red = 0;
    m_green = 0;
    m_blue = 0;
    m_brightness = 0;
    pinMode(m_redPin, OUTPUT);
    pinMode(m_greenPin, OUTPUT);
    pinMode(m_bluePin, OUTPUT);
    analogWriteResolution( 8);
    analogWriteFrequency(5000 );
    updateOuptutValues();
}

void LEDController::setNewState(JsonObject json)
{
    m_isOn = json["poweron"];
    m_red = json["red"];
    m_green = json["green"];
    m_blue = json["blue"];
    m_brightness = json["brightness"];
    updateOuptutValues();
}

JsonDocument LEDController::getCurrentState()
{
    JsonDocument jsonDocument;
    jsonDocument["poweron"] = m_isOn;
    jsonDocument["red"] = m_red;
    jsonDocument["green"] = m_green;
    jsonDocument["blue"] = m_blue;
    jsonDocument["brightness"] = m_brightness;
    return jsonDocument;
}

void LEDController::updateOuptutValues()
{
    if (m_isOn)
    {
        int calcRed = map(m_red, 0, 100, 0, m_brightness);
        int calcGreen = map(m_green, 0, 100, 0, m_brightness);
        int calcBlue = map(m_blue, 0, 100, 0, m_brightness);
        analogWrite(m_redPin, calcRed);
        analogWrite(m_greenPin, calcGreen);
        analogWrite(m_bluePin, calcBlue);
    }
    else
    {
        analogWrite(m_redPin, 0);
        analogWrite(m_greenPin, 0);
        analogWrite(m_bluePin, 0);
    }
}
