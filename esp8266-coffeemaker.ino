/**
 * Firmware for a simple esp8266 coffeemaker.
 * Based on Wemos D1 board
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>

MDNSResponder mdns;

// WiFi AP ssid/password
#include "src/wifi-credentials/wifi-credentials.h"

ESP8266WebServer server(80);

// Relay is connected to D8
int relay_pin = D8;
bool relay_state = false;

// Relay timeout
Ticker relay_timeout;

bool setRelayState(bool state) {
  relay_state = state;
  digitalWrite(relay_pin, relay_state ? HIGH : LOW);

  if(state)
    relay_timeout.once(60 * 10, [](){ setRelayState(false); });
  else
    relay_timeout.detach();
}

void setup(void){
  
  pinMode(relay_pin, OUTPUT);
  digitalWrite(relay_pin, LOW);

  delay(1000);
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  mdns.begin("esp8266", WiFi.localIP());

  SPIFFS.begin();
  server.serveStatic("/", SPIFFS, "/index.html");
  
  server.on("/api/state", []{
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/html", String(relay_state));
  });
  
  server.on("/api/on", []{
    setRelayState(true);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/html", String(relay_state));
  });
  
  server.on("/api/off", []{
    setRelayState(false);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/html", String(relay_state));
  });
  server.begin();
}
 
void loop(void){
  server.handleClient();
}
