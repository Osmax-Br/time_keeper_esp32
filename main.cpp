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

//oled screen init
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int p = 0;
//ui vars
int filled_rect = -1 ; //for inverting text
int table[12][2] = {{0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1}, {2, 1}, {0,2}, {1,2}, {2,2}, {0,3}, {1,3}, {2,3}}; 
const int pages = 4; // ui pages number
String str[pages][13] = {{"school","mosque","sleep","musiq","eat","anime","bath","out","face","utube","quran","study","Nothing"},
{"arabic","french","math","phys","chimst","scienc","draw","cf","python","esp32","book","minec","Nothing"},
{"tidy","fix","souq","/","/","/","famlyM","famlyF","edit","cook","/","/","Nothing"},
{"/","/","/","/","/","/","/","/","/","/","/","/","Nothing"}};
int rect_cord[12][4] = {{0,0,43,18},{42,0,43,18},{84,0,43,18},{0,17,43,18},{42,17,43,18},{84,17,43,18},{0,33,43,17},{42,33,43,17},{84,33,43,17},{0,49,43,15},{42,49,43,15},{84,49,43,15}};
int lastpress,press_state = 0;

//making button clases , each button is a spereate object
button_press button_up(35);
button_press button_down(34);
button_press button_right(33);
button_press button_left(32);
button_press button_selecT(27);
int last_press_time = 0; // for ignoring multiple presses at the same moment
String up,down,right,left,selecT = "" ; // strores button values
int up_bounce,down_bounce,righ_bounce,left_bounce,selecT_bounce = 0;  // stores bounce press values



void draw_grid(){
  for(int i = 0 ; i<12 ; i++){
    display.drawRect(rect_cord[i][0],rect_cord[i][1],rect_cord[i][2],rect_cord[i][3], WHITE);
  }
  
  
  
  /*
display.drawRect(0,0,43,18, WHITE);
display.drawRect(42,0,43,18, WHITE);
display.drawRect(84,0,43,18, WHITE);
//////////////////////////////////
display.drawRect(0,17,43,18, WHITE);
display.drawRect(42,17,43,18, WHITE);
display.drawRect(84,17,43,18, WHITE);
/////////////////////////////////
display.drawRect(0,33,43,17, WHITE);
display.drawRect(42,33,43,17, WHITE);
display.drawRect(84,33,43,17, WHITE);
///////////////////////////////
display.drawRect(0,49,43,15, WHITE);
display.drawRect(42,49,43,15, WHITE);
display.drawRect(84,49,43,15, WHITE);*/
}




void print_label(int current_page,int filled_rect) { //printing the activity inside its box in the grid
  display.setTextSize(1);
  int i2=0;
  for(int i=0;i<12;i++){  
  int x = table[i][0];
  int y = table[i][1];  
  if(i==9){
    i2 = -2;
  }
  display.setCursor(4 + 41 * x,3 + (17 * y) +i2);
  if(i == filled_rect && filled_rect != -1){
  display.setTextColor(BLACK);
  display.print(str[current_page][i]);}
  else{
 display.setTextColor(WHITE);
  display.print(str[current_page][i]);
  }
  }
}

void grid_navigiation(){
  if(selecT_bounce == 1){
    display.fillRect(0,0,43,18, WHITE);
    filled_rect = 0 ;
  }
  else if(selecT_bounce == 0){
    display.fillRect(0,0,43,18, BLACK);
    display.drawRect(0,0,43,18, WHITE);
     filled_rect = -1 ;
  }
print_label(0,filled_rect);
}



void setup() {
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);


 
 }

void loop() {
// adding the value of each button to a seperate var
up = button_up.press();
down = button_down.press();
right = button_right.press();
left = button_left.press();
selecT = button_selecT.press(); // the name is with "T" not "t" due to interferance with another library :p
// adding the values of bounce press vars
selecT_bounce = button_selecT.bounce_press();

draw_grid();
if(selecT_bounce == 1){
    display.fillRect(0,0,43,18, WHITE);
    filled_rect = 0 ;
  }
  else if(selecT_bounce == 0){
    display.fillRect(0,0,43,18, BLACK);
    display.drawRect(0,0,43,18, WHITE);
     filled_rect = -1 ;
  }
print_label(0,filled_rect);


Serial.print(button_selecT.bounce_press());



/*
draw_gridd();

if(selecT == 1){
  display.fillRect(0,0,43,18, WHITE);
  filled_rect = 0 ;
  
}
print_label(0,filled_rect);*/
display.display();
}
 