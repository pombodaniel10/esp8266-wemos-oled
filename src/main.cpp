#include <Arduino.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 

// WiFi includes
#include <ESP8266WiFi.h>

// Include the correct display library
// For a connection via I2C using Wire include
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server = "192.168.1.53";


const long utcOffsetInSeconds = -18000;

const String daysOfTheWeek[7] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};


// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "south-america.pool.ntp.org", utcOffsetInSeconds);

// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, 5, 4);

unsigned int printTimeInterval = 1000;
unsigned long printTime;

String mensaje; 

void callback(char* topic, byte* payload, unsigned int length);

String twoDigits(int digits){
  if(digits < 10) {
    String i = '0'+String(digits);
    return i;
  }
  else {
    return String(digits);
  }
}

void setup() {

  Serial.begin(115200);

  display.init();
  display.flipScreenVertically();
  display.setContrast(255);
  // Align text vertical/horizontal center
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.setFont(ArialMT_Plain_10);

  WiFiManager wifiManager;
  //wifiManager.resetSettings();

  display.drawString(display.getWidth()/2, 10 , "AP ENABLED");
  display.drawString(display.getWidth()/2, 20 , "AP ENABLED");
  display.drawString(display.getWidth()/2, 30 , "AP ENABLED");
  display.drawString(display.getWidth()/2, 40 , "AP ENABLED");
  display.drawString(display.getWidth()/2, 50 , "AP ENABLED");
  display.drawString(display.getWidth()/2, 60 , "AP ENABLED");
  display.display();

  wifiManager.autoConnect("AutoConnectAP");
  
  display.clear();

  display.drawString(display.getWidth()/2, display.getHeight()/2, "WiFi Connected:\n" + WiFi.SSID() + "\n" + WiFi.localIP().toString());
  display.display();

  printTime = millis();
  delay(5000);

  timeClient.begin();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  mensaje = "";
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    mensaje += (char)payload[i];
  }
  Serial.println();
}
 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("prueba", "Enviando el primer mensaje");
      // ... and resubscribe
      client.subscribe("prueba");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  if(millis()-printTime>=printTimeInterval){

    printTime = millis();
    timeClient.update();
    display.clear();

    Serial.print(daysOfTheWeek[timeClient.getDay()]);
    Serial.print(", ");
    Serial.print(timeClient.getHours());
    Serial.print(":");
    Serial.print(timeClient.getMinutes());
    Serial.print(":");
    Serial.println(timeClient.getSeconds());

    String timenow = String(timeClient.getHours())+":"+twoDigits(timeClient.getMinutes())+":"+twoDigits(timeClient.getSeconds());

    display.drawString(display.getWidth()/2, 10, "32 °C");
    display.drawString(display.getWidth()/2, 20, daysOfTheWeek[timeClient.getDay()]);
    display.drawString(display.getWidth()/2, 30, timenow);
    display.drawString(display.getWidth()/2, 50, mensaje);
    display.display();

  }
}