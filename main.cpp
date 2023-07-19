#include <Arduino.h>
#include "ESPAsyncWebServer.h"
#include <press.h> // for multiple buttons regestration
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <Keypad.h>
#include <NTPClient.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFiUdp.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
//making button clases , each button is a spereate object
button_press button_up(35);
button_press button_down(34);
button_press button_right(33);
button_press button_left(32);
button_press button_selecT(27);
int last_press_time = 0;
String up,down,right,left,selecT = "" ; // strores button values

void setup() {
  Serial.begin(115200);
 
 }

void loop() {
up = button_up.press();
Serial.print(up);

down = button_down.press();
Serial.print(down);

right = button_right.press();
Serial.print(right);

left = button_left.press();
Serial.print(left);

selecT = button_selecT.press();
Serial.print(selecT);
}
 