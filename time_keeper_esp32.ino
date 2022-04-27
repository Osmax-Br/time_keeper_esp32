#include "ESPAsyncWebServer.h"
#include <ETH.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiSTA.h>
#include <WiFiType.h>
#include <WiFiUdp.h>
#include <Keypad.h>
#include <NTPClient.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFiUdp.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ThingSpeak.h"
#include "html.h" 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
IPAddress local_IP(192, 168, 1, 199); //192.168.1.199
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional
AsyncWebServer server(80);

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "Timer"){
    return timer_var();
  }
  else if(var == "Chosen_server"){
    return chosen_var();
    }
  else if(var == "Pause"){
    return pause_var();
    }  
  return String();
}



int table[12][2] = {{0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1}, {2, 1}, {0,2}, {1,2}, {2,2}, {0,3}, {1,3}, {2,3}};
const int activities = 4;
String str[activities][13] = {{"school","mosque","sleep","musiq","eat","anime","bath","out","face","utube","quran","study","Nothing"},
{"arabic","french","math","phys","chimst","scienc","draw","cf","python","esp32","book","minec","Nothing"},
{"tidy","fix","souq","/","/","/","famlyM","famlyF","edit","cook","/","/","Nothing"},
{"musiq","eat","anime","/","/","/","phys","chimst","scienc","cook","famlyF","edit","Nothing"}};
const long utcOffsetInSeconds = 7200;
int hours=0;
String day = "???";
int minutes = 0;
int seconds = 0;
String year = "0000";
String dayStamp = "00-00";
char amPm;
char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
String formattedDate;
const char *ssid     = "phb";
const char *password = "phb@1973";
#define ROW_NUM     4
#define COLUMN_NUM  4
int chosen = -1;
int key_num = -48;
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'D'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'B'}
};
unsigned long myChannelNumber = 1;
const char * myWriteAPIKey = "1B8PQM7MAYAT0WIO";
// 5 --> 13
//18 --> 12
// 19 --> 14
byte pin_rows[ROW_NUM]      = {19, 18, 5, 17};
byte pin_column[COLUMN_NUM] = {16, 4, 0, 2};
int a,b,c,d,square,saved,startTimer,lastPress,lastUpdate =0 ;
int start_hour,start_minute,start_second,end_hour,end_minutes_end_second = 0;
int pressed = -1;
WiFiClient  clientt;
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
String wrd;
hw_timer_t * timer = NULL;
long sec,minu,hor;
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;
String new_sec,new_minu,new_hor = "00";
int saving = -1;
String saved_data[20];
void IRAM_ATTR onTimer(){
  if(startTimer == 1){
  sec+=1;
  if(sec>=59){
    sec=0;
    minu++;
    }
  if(minu>=59){
    minu=0;
    hor++;
    } }
   else if(startTimer ==0){
   sec = 0;
   hor =0;
   minu = 0;
   } 
   beautiful_int(sec,hor,minu);
 }
 
 String timer_var(){
 return String(new_hor)+":"+String(new_minu)+":"+String(new_sec);
  }
String chosen_var(){
    if(chosen==-1){
      return "Nothing";
    }
  else{
    return str[square][chosen] ;
    }}
String pause_var(){
  if(startTimer==1){
    return "Pause";
    }
  else if(startTimer == 2){
    return "Resume";
    }
   else{
    return "Start";
    }}    
  
void setup() {
    Serial.begin(9600);
if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
     if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect");
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password);  
        break;    
      } 
      Serial.println("\nConnected.");
    }
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(0);
  display.setTextColor(WHITE);
  timeClient.begin();
  timer = timerBegin(0, 80, true);  // timer 0, MWDT clock period = 12.5 ns * TIMGn_Tx_WDT_CLK_PRESCALE -> 12.5 ns * 80 -> 1000 ns = 1 us, countUp
  timerAttachInterrupt(timer, &onTimer, true); // edge (not level) triggered 
  timerAlarmWrite(timer, 1000000, true); // 1000000 * 1 us = 1 s, autoreload true
  timerAlarmEnable(timer); // enable
  ThingSpeak.begin(clientt);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  
  server.on("/timer", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", timer_var().c_str());
  });
   server.on("/chosen_server", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", chosen_var().c_str());
  });
    server.on("/Pause", HTTP_GET, [] (AsyncWebServerRequest *request) {
    Serial.println("Resume");
    request->send(200, "text/plain", "ok");
  });
  server.on("/btn", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", pause_var().c_str());
  });

  // Receive an HTTP GET request
  server.on("/Resume", HTTP_GET, [] (AsyncWebServerRequest *request) {
    Serial.println("Pause");
    request->send(200, "text/plain", "ok");
  });
  server.begin();
}

void loop() { 
if(Serial.available()) {
  String dd = Serial.readStringUntil('T');
  if(dd == "off"){
    WiFi.disconnect();
    }
  if(dd == "on"){
    WiFi.begin(ssid, password);
    }  
}
char key = keypad.getKey();  
if(key){
  display.ssd1306_command(SSD1306_DISPLAYON);
  lastPress = millis();
  }  
if(lastPress+30000<millis()){
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  } 
if(key == 'C'){
  if(square==activities-1){
      square = 0;}
  else{
    square+=1;}}   
if(key=='A' and a!=1) {
  display.clearDisplay();
  

  print_label(key);
  a=1;
  key = '?';
}
else if(key=='A' and a==1){
display.clearDisplay();
display.display();
a=0;}
if(a==1){
edit_key(key);
if(key_num != -48){
    chosen = key_num-1;
  }}

  
  
if(a!=1){
    main_screen(chosen,hor,minu,sec,key,new_hor,new_minu,new_sec);
   drawer();  
 }
  
display.display();
}










void drawer(){
  display.drawLine(0, 10, display.width() - 1, 10, WHITE);  
  }


void main_screen(int chosen,int hor,int minu,int sec,char key,String new_hor,String new_minu,String new_sec ){
 display.clearDisplay();
   display.setCursor(0,40+15);
  display.setTextSize(1);
  display.print("choice: ");
  if(chosen==-1){
    display.print("Nothing");
    }
  else{
    display.print(str[square][chosen]);
    } 
 String ampm;
 if(WiFi.status() != WL_CONNECTION_LOST && WiFi.status()== WL_CONNECTED){ 
  if(lastUpdate+120000<millis()){
 timeClient.update();
 lastUpdate = millis();
 Serial.println("update!");
 }
 else if(millis()<10000){
  timeClient.update();
  }}
 formattedDate = timeClient.getFormattedDate();
 int splitT = formattedDate.indexOf("T");
 dayStamp = formattedDate.substring(5, splitT);
 hours=timeClient.getHours();
 day = daysOfTheWeek[timeClient.getDay()];
 minutes = timeClient.getMinutes();
 seconds = timeClient.getSeconds();
 year = formattedDate.substring(0,4);
 if(hours>12){
  hours-=12;
  ampm = "Pm";}
 else{
 ampm = "Am";
 }
 if(year == "1970"){
  hours = minutes = seconds = 0;
  day = "???";
  dayStamp = "0-0";
  }

 int clearr = millis();
 
 
 
 display.setCursor(17,0+15);
 display.setTextSize(2);
 char bufffer[40];
 sprintf(bufffer, "%s:%s:%s",new_hor,new_minu,new_sec);

if (key=='B' and b==0) {
  start_hour = hours;
  start_minute = minutes;
  start_second = seconds;
  c=1;
  b=1;
  startTimer = 1;
  key = '1';
  pressed+=1;
}
else if(key=='B' and b==1){
display.clearDisplay();
display.display();
b=0;
c=0;
key = '1';
}

if(key=='*' and d==0){
display.clearDisplay();
display.display();  
d=1;
c=2;
key = '1';
  }
else if(key=='*' and d==1){
  d=0;
  c=1;
  }  
if(c==0){
  if(pressed >= 0 and chosen!=-1){
  //display.clearDisplay();
  display.print("Saving..");
  if ((millis() - lastTime) > timerDelay){
    char datum[200];
    sprintf(datum, "%s/%s/%i:%i:%i/%i:%i:%i/%i:%i:%i",str[square][chosen],dayStamp,start_hour,start_minute,start_second,new_hor,new_minu,new_sec,hours,minutes,seconds);
    if(WiFi.status() != WL_CONNECTION_LOST && WiFi.status()== WL_CONNECTED){
    int x = ThingSpeak.writeField(myChannelNumber, 1,datum, myWriteAPIKey);
    Serial.println(datum);
    if(x == 200){
       display.println("saved!"); 
       delay(500);}
    else{
      Serial.println("error");
    }}
    else{
      saving+=1;
      char datum[200];
      sprintf(datum, "%s/%s/%i:%i:%i/%i:%i:%i/%i:%i:%i",str[square][chosen],dayStamp,start_hour,start_minute,start_second,new_hor,new_minu,new_sec,hours,minutes,seconds);
      saved_data[saving]=datum;
      display.print("saved to var");
      Serial.println(saved_data[0]);
      Serial.println(saved_data[1]);
      Serial.println(saved_data[2]);
      delay(500);
      }}
      lastTime = millis();
     pressed = -1;
      }
  else{
  display.print("00:00:00");}
  startTimer = 0;

}
else if(c==1){
  display.print(bufffer);
  startTimer = 1;
}

else if(c==2){
  display.print(bufffer);
  startTimer = 2;
}

 display.setTextSize(1);
 display.setCursor(0,20+15);
 char buffer[40];
 sprintf(buffer,"%i:%i:%i %s %s %s",hours,minutes,seconds,ampm,day,dayStamp);
 display.print(buffer);
 int savings = saving;
 if(saving>-1 and WiFi.status() != WL_CONNECTION_LOST && WiFi.status()== WL_CONNECTED){
  display.print("saving");
      for(int i=0;i<savings+1;){
        if((millis() - lastTime) > timerDelay){
          Serial.print("saving ");
          Serial.println(saved_data[i]);
      int x = ThingSpeak.writeField(myChannelNumber,1,saved_data[i], myWriteAPIKey);
      if(x==200){
       Serial.println("saving sucess");
       i++;
       saving -=1;
       }
       else{
        Serial.println("error");
        }
       lastTime = millis();
       }
      }
      }
 
  }
        


void print_label(char key) {
  display.setTextSize(1);
  draw_grid();
  for(int i=0;i<12;i++){  
  int x = table[i][0];
  int y = table[i][1];  
  display.setCursor(2 + 41 * x, 4 + 16 * y);
  display.setTextColor(WHITE);
  display.print(str[square][i]);
  }
}
void draw_grid() {
  display.drawLine(0, 0, 0, display.height() - 1, WHITE);
  display.drawLine(41, 0, 41, display.height() - 1, WHITE);
  display.drawLine(81, 0, 81, display.height() - 1, WHITE);
  display.drawLine(127, 0, 127, display.height() - 1, WHITE);
  for (int i = 0; i <= 64 - 15; i += 16) {
    display.drawLine(0, i, display.width() - 1, i, WHITE);
  }
  display.drawLine(0, 63, display.width() - 1, 63, WHITE);}
  
void keypress() {
display.setCursor(0, 0);
char key = keypad.getKey();

if (key) {
if (key != '*') {
display.clearDisplay();
wrd += key;
display.print(wrd);
Serial.println(wrd);
}
else {
  Serial.println(wrd);
  display.clearDisplay();
  int len;
  len = wrd.length();
  wrd = wrd.substring(0, len - 1);
  Serial.println(wrd);
  display.print(wrd);
}
display.display();
}}


void edit_key(char key){
  if(key == '*'){
  key_num = 10;
  }
else if(key == '0'){
  key_num = 11;  
}
else if(key=='#'){
  key_num = 12;
}
else if(key=='?'){
  key_num = 13;
  }
else{
 key_num= key-'0';}
}


void beautiful_int(int sec,int hor,int minu){
  if(sec<10){
    new_sec = "0" + String(sec);
    }
  else{
    new_sec = String(sec);
    }  
  if(minu<10){
    new_minu = "0" + String(minu);
    }
  else{
    new_minu = String(minu);
    }  
    if(hor<10){
    new_hor = "0" + String(hor);
    }
  else{
    new_hor = String(hor);
    }
  }  
 /*
  *  const btn = document.getElementById('btn');
btn.addEventListener('click', function handleClick() {
const initialText = 'Pause';

  if (btn.textContent.toLowerCase().includes(initialText.toLowerCase())) {
    btn.textContent = 'Resume';
     var xhr = new XMLHttpRequest();
     xhr.open("GET", "/" + 'Resume', true);
     xhr.send();
  } else {
    btn.textContent = initialText;
     var xhr = new XMLHttpRequest();
     xhr.open("GET", "/" + initialText, true);
     xhr.send();
  }});
  */ 
