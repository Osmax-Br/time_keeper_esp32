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
//ui vars
String chosen_value = "Nothing"; //the chosen value from the grid
int select_mode = 0 ; //for maniging selection ways
int filled_rect = -1 ; //for inverting text
int table[12][2] = {{0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1}, {2, 1}, {0,2}, {1,2}, {2,2}, {0,3}, {1,3}, {2,3}}; //for storing the coordinats of each square in the grid
const int pages = 4; // ui pages number
int current_page = 0; // for navigation
String str[pages][13] = {{"school","mosque","sleep","musiq","eat","anime","bath","out","face","utube","quran","study","Nothing"},
{"arabic","french","math","phys","chimst","scienc","draw","cf","python","esp32","book","minec","Nothing"},
{"tidy","fix","souq","/","/","/","famlyM","famlyF","edit","cook","/","/","Nothing"},
{"/","/","/","/","/","/","/","/","/","/","/","/","Nothing"}};
int rect_cord[4][3][4] = {{{0,0,43,18},{42,0,43,18},{84,0,43,18}},{{0,17,43,18},{42,17,43,18},{84,17,43,18}},{{0,33,43,17},{42,33,43,17},{84,33,43,17}},{{0,49,43,15},{42,49,43,15},{84,49,43,15}}};  //the data of each rectangular in the grid eg: width,height,x,y
int lastpress,press_state = 0; //for button class
int cursor[2] = {0,0} ; //the cursor for grid the bottom-left rect is the 0,0 

//making button clases , each button is a spereate object
button_press button_up(35);
button_press button_down(34);
button_press button_right(33);
button_press button_left(32);
button_press button_selecT(27);
int last_press_time = 0; // for ignoring multiple presses at the same moment
String up,down,right,left,selecT = "" ; // strores button values
int up_bounce,down_bounce,righ_bounce,left_bounce,selecT_bounce = 0;  // stores bounce press values



void draw_grid(){ //used this method for easier tex inverting 
  for(int i = 0 ; i<4 ; i++){
    for(int j=0 ; j<3 ; j++){
    display.drawRect(rect_cord[i][j][0],rect_cord[i][j][1],rect_cord[i][j][2],rect_cord[i][j][3], WHITE);
  }}
}




void print_label(int current_page,int filled_rect_x , int filled_rect_y) { //printing the activity inside its box in the grid
  draw_grid();
  display.setTextSize(1);
  int i2=0; //this is to adjust the y shift while printing labels
  for(int i=0;i<12;i++){  
  int x = table[i][0]; // getting the coordintaes of each rectangular
  int y = table[i][1];  
  if(i==9){
    i2 = -2;  //adjusting y shift
  }
  display.setCursor(4 + 41 * x,3 + (17 * y) +i2); //setting writing cursor
  if(x == filled_rect_x && y==filled_rect_y){ //&& selecT_bounce == 1){   //this is for inverting the text color while selecting
  display.setTextColor(BLACK);
  display.print(str[current_page][i]);} //printing the labels
  else{
 display.setTextColor(WHITE);
  display.print(str[current_page][i]);
  }
  }
}

void grid_navigiation(){
  select_mode = 1;
    int x = cursor[1]; // getting current cursor coordintes
    int y = cursor[0];
  if(right == "pressed"){
    if(cursor[0] + 1 != 3){ //this is for checking the the cursor doesnt go out of the screen (stops at the edges)
    cursor[0] = cursor[0] + 1; // adjusting x cursor
    display.clearDisplay(); // clearing display for interferance
  }}

   if(left == "pressed"){
    if(cursor[0] -1  != -1){
    cursor[0] = cursor[0] - 1; 
    display.clearDisplay();
  }} 
   if(up == "pressed"){
    if(cursor[1] -1  != -1){
    cursor[1] = cursor[1] - 1; 
    display.clearDisplay();
  }} 
   if(down == "pressed"){
    if(cursor[1] +1  != 4){
    cursor[1] = cursor[1] + 1 ; 
    display.clearDisplay();
  }} 
  
  
  //if(selecT_bounce == 1){
    display.clearDisplay(); //without it the lables will over lap
    display.fillRect(rect_cord[x][y][0],rect_cord[x][y][1],rect_cord[x][y][2],rect_cord[x][y][3], WHITE); //making the rectangular white
  //}
  //else if(selecT_bounce == 0){
   // display.clearDisplay();
    //display.fillRect(rect_cord[x][y][0],rect_cord[x][y][1],rect_cord[x][y][2],rect_cord[x][y][3], BLACK);
    //display.drawRect(rect_cord[x][y][0],rect_cord[x][y][1],rect_cord[x][y][2],rect_cord[x][y][3], WHITE); //making the rectangualr normal again
 // }


if(selecT == "pressed"){ //the action which happens when something is selected
int x = cursor[0]; //the same code used in the function print_label
int y = cursor[1]; //the code is mainly to change from coordinates in cusor to a number which allows us to know the value selected
for(int i=0 ; i<12 ; i++){
  int x1 = table[i][0];
  int y1 = table[i][1];
  if(x1==x && y==y1){
    chosen_value = str[current_page][i]; //the Actual selected value 
    Serial.print(str[current_page][i]);
    //select_mode = 0;
    break;
  }
}

}

if(right == "long_pressed" && current_page != 3){ //for flipping pages and ensuring that an error doesnt happen
  current_page +=1 ;   // change page
}
if(left == "long_pressed" && current_page != 0){
  current_page -= 1;
}

print_label(current_page,cursor[0],cursor[1]); // printing the lables
}



void main_screen(String chosen_value){
  display.clearDisplay();
  display.setCursor(0,40+15);
  display.setTextSize(1);
  display.print("choice: ");
  if(chosen_value=="Nothing"){
    display.print("Nothing");
    }
  else{
    display.print(chosen_value);
    } 
  display.setCursor(17,0+15);
  display.setTextSize(2);
  display.print("00:00:00");
  display.setTextSize(1);
  display.setCursor(0,20+15);
  display.print("12:34:12 AM wed 14/12");

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

if(selecT == "long_pressed"){
  if(select_mode == 1){
    select_mode = 0;
  }
  else if(select_mode == 0){
    select_mode = 1;
  }
}


if (select_mode == 1){
  select_mode = 0;
  grid_navigiation(); // the grid navigiation & selectin & invertion function
}
else if(select_mode == 0){
  main_screen(chosen_value);
}


display.display();
}
 