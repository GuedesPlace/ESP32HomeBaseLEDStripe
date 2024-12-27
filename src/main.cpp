#include <Arduino.h>
#include <AsyncUDP.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <AsyncMessagePack.h>
#include <Preferences.h>
#include "./utils.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <GPBleWrapper.h>
#include <GPWiFiWrapper.h>
#include "LEDController.h"

#define LED 2
#define REDPIN 12
#define GREENPIN 14
#define BLUEPIN 5

// TODO:
//  Broadcast Name and status "UP", nothing else
//  Setup Async WebServer on Port 80 with endpoint /api/led (PUT/GET)

void broadcastStatus();
void checkWiFiSetupOnStart();

Preferences preferences;
AsyncUDP udp;

LEDController *ledController;

GPBleWrapper bleWrapper;
GPWifiWrapper wifiWrapper;

AsyncCallbackJsonWebHandler *jsonHandler = new AsyncCallbackJsonWebHandler("/api/led");
AsyncWebServer server(80);
boolean serverIsRunning = false;
String publicBroadcastName = "";

class MyEspUpdateCallbacks : public GPBleUpdateValueCallbacks
{
public:
  MyEspUpdateCallbacks(GPWifiWrapper *wrapper)
  {
    m_wrapper = wrapper;
  }
  virtual void onWifiCredentialsUpdate(String ssid, String password)
  {
    preferences.begin("esp32LEDStripe", false);
    preferences.putString("ssid", ssid);
    preferences.putString("pw", password);
    preferences.end();
    m_wrapper->updateWifiCredentials(ssid, password);
  };
  String getSSID()
  {
    preferences.begin("esp32LEDStripe", true);
    String ssid = preferences.getString("ssid", "");
    preferences.end();
    return ssid;
  };
  String getPassword()
  {
    preferences.begin("esp32LEDStripe", true);
    String pw = preferences.getString("pw", "");
    preferences.end();
    return pw;
  };
  GPConfigurationProperties *getProperties()
  {
    GPConfigurationProperties *properties = new GPConfigurationProperties(1);
    preferences.begin("esp32LEDStripe", false);
    String publicName = GetHostName();
    if (preferences.isKey("publicName"))
    {
      publicName = preferences.getString("publicName", GetHostName());
    }
    preferences.end();
    gpConfigProperty propertyPN = {"pn", "Public Name for this LED Stripe", publicName};
    properties->setProperty(propertyPN, 0);
    return properties;
  };
  void onConfigPropertyUpdate(GPConfigurationProperties *properties)
  {
    preferences.begin("esp32LEDStripe", false);
    String publicName = properties->getProperty(0).value;
    preferences.putString("publicName", publicName);
    preferences.end();
    publicBroadcastName = publicName;
  };

private:
  GPWifiWrapper *m_wrapper;
};

class MyWiFiStatusCallBack : public GPWifiStatusCallbacks
{
public:
  MyWiFiStatusCallBack(GPBleWrapper *wrapper)
  {
    m_wrapper = wrapper;
  }
  void onWifiStatusUpdate(GPBLE_WIFISTATUS wifiStatus)
  {
    m_wrapper->updateWIFIStatus(wifiStatus);
  }

private:
  GPBleWrapper *m_wrapper;
};

void setup()
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  delay(1000);
  ledController = new LEDController(REDPIN, GREENPIN, BLUEPIN);
  String deviceName = GetHostName();
  Serial.print("Device: ");
  Serial.println(deviceName);
  bleWrapper.setup(deviceName);
  Serial.print("Setup done.. ");
  bleWrapper.registerUpdateValueCallBacks(new MyEspUpdateCallbacks(&wifiWrapper));
  wifiWrapper.setup(deviceName);
  Serial.print("WIFI Setup done.. ");
  wifiWrapper.registerWifiStatusCallbacks(new MyWiFiStatusCallBack(&bleWrapper));
  Serial.print("PreCHECK.. ");
  server.on("/api/led", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    AsyncResponseStream* response = request->beginResponseStream("application/json");
    JsonDocument doc = ledController->getCurrentState();
    serializeJson(doc, *response);
    request->send(response); });
  jsonHandler->setMethod(HTTP_PUT);
  jsonHandler->onRequest([](AsyncWebServerRequest *request, JsonVariant &json)
                         {
    ledController->setNewState(json.as<JsonObject>());
    AsyncResponseStream* response = request->beginResponseStream("application/json");
    JsonDocument doc = ledController->getCurrentState();
    serializeJson(doc, *response);
    request->send(response); });
  server.addHandler(jsonHandler);
  checkWiFiSetupOnStart();
}

void loop()
{
  delay(2000);
  wifiWrapper.checkStatus();
  if (wifiWrapper.isConnected())
  {
    broadcastStatus();
  }
  if (!serverIsRunning && wifiWrapper.isConnected())
  {
    server.begin();
  }
  if ((serverIsRunning && !wifiWrapper.isConnected()))
  {
    server.end();
  }
}
void broadcastStatus()
{
  JsonDocument doc;
  doc["status"] = "up";
  doc["publicName"] = publicBroadcastName;
  String message;
  serializeJson(doc, message);
  char messageChar[message.length() + 1];
  message.toCharArray(messageChar, message.length() + 1);
  udp.broadcastTo(messageChar, 8266);
}
void checkWiFiSetupOnStart()
{
  delay(2000);
  preferences.begin("esp32LEDStripe", true);
  if (preferences.isKey("ssid") && preferences.isKey("pw"))
  {
    String ssid = preferences.getString("ssid", "");
    String pw = preferences.getString("pw", "");
    if (!ssid.isEmpty())
    {
      Serial.print("Try to connect to WLAN: ");
      Serial.println(ssid);
      wifiWrapper.updateWifiCredentials(ssid, pw);
    }
    else
    {
      Serial.println("No SSID for WLAN defined!");
    }
  }
  if (preferences.isKey("publicName"))
    {
      publicBroadcastName = preferences.getString("publicName", GetHostName());
    }
  preferences.end();
}