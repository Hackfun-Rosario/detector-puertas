
// Load Wi-Fi library
#include <WiFi.h>
#include "WiFiClient.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <ArduinoJson.h>

#include "PubSubClient.h"

#define DOOR_SENSOR_PIN 19 // ESP32 pin GPIO19 connected to door sensor's pin

// Replace with your network credentials

const char *ssid = "";
const char *password = "";

// Variables
int doorState;
int doorPrevState = HIGH;
// char str[2];

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

WiFiClient espClient;
PubSubClient clientMqtt(espClient);

void callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("myMQTT::incoming ");
  Serial.print(topic);
  Serial.println(" ");

  StaticJsonDocument<200> jsonMessage;

  if (topic != "door/trigger")
  {

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(jsonMessage, message);

    // Test if parsing succeeds.
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    // Fetch values.
    //
    // Most of the time, you can rely on the implicit casts.
    // In other case, you can do doc["time"].as<long>();
    const char *door = jsonMessage["door"];

    // Print values.

    if (door != "reScan")
    {

      Serial.println("Entra al if");
      Serial.println("doorState");
      Serial.println(doorState);

      clientMqtt.publish("door/status", doorState == HIGH ? "{\"open\":false}" : "{\"open\":true}", true);
    }
  }

  // else
  // {
  //   Serial.println(messageTemp.compareTo(reScan));
  // }
}

void setup()
{
  Serial.begin(115200);
  Serial.print("Start to connect ");

  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP); // set ESP32 pin to input pull-up mode

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  clientMqtt.setServer();
  clientMqtt.setCallback(callback);

  while (!clientMqtt.connected())
  {
    if (clientMqtt.connect(""))
    {
      Serial.println(" connected");

      // Subscribe
      clientMqtt.subscribe("door/trigger");
    }
    else
    {
      Serial.print("myMQTT::WaitForConnection... failed, rc=");
      Serial.print(clientMqtt.state());
      Serial.println(" try again in 60 seconds, ");
      // Wait 5 seconds before retrying
      // delay(5000);
    }
  }
}

void loop()
{

  clientMqtt.loop();

  doorState = digitalRead(DOOR_SENSOR_PIN); // read state

  if (doorState != doorPrevState)
  {
    Serial.print("doorPrevState ");
    Serial.println(doorPrevState);
    Serial.println("");

    doorPrevState = doorState;
    // modo incorrecto
    // String open = String(doorPrevState);

    // modo correcto a mano de chino
    char msg[32];
    sprintf(msg, "{\"open\":%i}", doorPrevState);

    clientMqtt.publish("door/status", doorState == HIGH ? "{\"open\":false}" : "{\"open\":true}");

    // ___________________________________________________

    // correcto correcto : usar lib arduinojson

    // Allocate the JSON document
    //
    // Inside the brackets, 200 is the RAM allocated to this document.
    // Don't forget to change this value to match your requirement.
    // Use https://arduinojson.org/v6/assistant to compute the capacity.
    // StaticJsonDocument<200> doc;

    // StaticJsonObject allocates memory on the stack, it can be
    // replaced by DynamicJsonDocument which allocates in the heap.
    //
    // DynamicJsonDocument  doc(200);

    // Add values in the document
    //
    // doc["sensor"] = "gps";
    // doc["time"] = 1351824120;

    // Add an array.
    //
    // JsonArray data = doc.createNestedArray("data");
    // data.add(48.756080);
    // data.add(2.302038);

    // Generate the minified JSON and send it to the Serial port.
    // String jsonDoc;

    // serializeJson(doc, jsonDoc);
    // The above line prints:
    // {"sensor":"gps","time":1351824120,"data":[48.756080,2.302038]}

    // Start a new line
    // Serial.println(jsonDoc);

    // Generate the prettified JSON and send it to the Serial port.
    //
    // serializeJsonPretty(doc, Serial);
    // The above line prints:
    // {
    //   "sensor": "gps",
    //   "time": 1351824120,
    //   "data": [
    //     48.756080,
    //     2.302038
    //   ]
    // }

    // clientMqtt.publish("door/status", jsonDoc.c_str());
  }
}