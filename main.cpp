#include "ESPAsyncWebServer.h"
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <HTTPClient.h>
#include <Arduino.h>
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
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <SPI.h>
#include <FS.h>
#include "SD.h"
#include "sd_card_core.h"
#include <IRremote.h>
#define SD_CS 23
#define SD_SCK 17
#define SD_MOSI 12
#define SD_MISO 13
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// local ip config for lan server
IPAddress local_IP(192, 168, 1, 199); //192.168.1.199
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional
AsyncWebServer server(80);
String serverName = "http://codeforces.com/api/contest.list"; //cf contest list api
unsigned long lastTime_cf,last_led = 0 ; //last time cf lists was updated
int cf_updated,ntp_updated,led_begin = 0;  // 0 not updated since startup otherwise it is updated
int array_item_uni = 1;
int temp_cf[11];
// cf data lists where we store cf data
int ContestTime [9] = {-1,-1,-1,-1,-1,-1,-1,-1,-1};
String ContestDiv [9] = {"Div -1","div -1","div -1","div -1","div -1","div -1","div -1","div -1","div -1"};
String ContestPhase [9] = {"na","na","na","na","na","na","na","na","na"};
String irkey = ""; // stores ir value
String irCode[4][2] = {{"f20d4040","status"},{"e9164040","pause"},{"e8174040","resume"},{"f50a4040","upload"}}; // ir value and its functionality
int table[12][2] = {{0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1}, {2, 1}, {0,2}, {1,2}, {2,2}, {0,3}, {1,3}, {2,3}}; // to show labels correctly when A is pressed
const int activities = 4; // number of pages when A is pressed
String str[activities][13] = {{"school","mosque","sleep","musiq","eat","anime","bath","out","face","utube","quran","study","Nothing"},
{"arabic","french","math","phys","chimst","scienc","draw","cf","python","esp32","book","minec","Nothing"},
{"tidy","fix","souq","/","/","/","famlyM","famlyF","edit","cook","/","/","Nothing"},
{"/","/","/","/","/","/","/","/","/","/","/","/","Nothing"}};
const long utcOffsetInSeconds = 7200+3600;
int hours=0;
String day_var = "???";
int minutes = 0;
int seconds = 0;
String year_var = "0000";
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
const int RECV_PIN = 15;
//IrReceiver IrReceiver(RECV_PIN);

//decode_results results;
byte pin_rows[ROW_NUM]      = {32, 18 , 5, 25};
byte pin_column[COLUMN_NUM] = {16, 4, 0, 2};
int pressA,pressB,pressD,counter_var,pressStar,square,saved,startTimer,lastPress,lastUpdate,cf_square,cf_tries =0 ;
int start_hour,start_minute,start_second,end_hour,end_minutes_end_second = 0;
int pressed = -1;
WiFiClient  clientt;
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
String wrd;
hw_timer_t * timer = NULL;
long sec_var,minu,hor;
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;
String new_sec,new_minu,new_hor = "00";
int saving = -1;
String saved_data[20];
const char* database_init = "CREATE table if not exists times(begin_hour int,  begin_minute int,  end_hour int,  end_minute int,  hours int,  minutes int,  seconds int,chosen string);";


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
void web_server(){
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
    request->send(200, "text/plain", "ok");
  });
  server.on("/btn", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", pause_var().c_str());
  });
  server.on("/Resume", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "ok");
  });
   server.on("/PauseBtn", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(pressStar==0){
      pressStar=1;
      counter_var=2;
      }
    else if(pressStar==1){
      pressStar=0;
      counter_var=1;
      }  
    
    request->send(200, "text/plain", "ok");
  });
  server.on("/choseee", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage = "";
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam("input1")) {
      inputMessage = request->getParam("input1")->value();
      inputParam = "input1";
      
    if(chosen < 12 && chosen != -1 && chosen != 12 ){
     chosen +=1 ;
    }
    else if(chosen == -1 || chosen == 12){
     for(int i =0 ; i<12 ;i++){
      if(str[3][i] == "/"){
        chosen = i;
        break;
     }
     else{
     chosen = 0;
     }}
    }
    else{
      chosen = 0;
    }
    square = 3 ;
    str[3][chosen] = inputMessage;}
    request->send(200, "text/html", " <meta http-equiv='refresh' content='0; URL=http://192.168.1.199/'> ");
  });
 
  server.begin();
}



// draws a line
void drawer(){
  display.drawLine(0, 10, display.width() - 1, 10, WHITE);  
  }



        
//draws the grid when A is pressed
void draw_grid() {
  display.drawLine(0, 0, 0, display.height() - 1, WHITE);
  display.drawLine(41, 0, 41, display.height() - 1, WHITE);
  display.drawLine(81, 0, 81, display.height() - 1, WHITE);
  display.drawLine(127, 0, 127, display.height() - 1, WHITE);
  for (int i = 0; i <= 64 - 15; i += 16) {
    display.drawLine(0, i, display.width() - 1, i, WHITE);
  }
  display.drawLine(0, 63, display.width() - 1, 63, WHITE);}
  
void print_label(int square) {
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

void keypress() {
display.setCursor(0, 0);
char key = keypad.getKey();

if (key) {
if (key != '*') {
display.clearDisplay();
wrd += key;
display.print(wrd);
}
else {
  
  display.clearDisplay();
  int len;
  len = wrd.length();
  wrd = wrd.substring(0, len - 1);
  
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


void beautiful_int(int sec_var,int hor,int minu){
  if(sec_var<10){
    new_sec = "0" + String(sec_var);
    }
  else{
    new_sec = String(sec_var);
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

void resume_tone(int ledPin){
  int ledState = LOW;             // ledState used to set the LED
  unsigned long previousMillis = 0;
  const long interval = 80;
  for(int i =0 ; i<4 ;){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;}
    digitalWrite(ledPin, ledState);
    i++;}}
  }

void pause_tone(int ledPin){
  int ledState = 0;             // ledState used to set the LED
  unsigned long previousMillis = 0;
  const long interval = 220;
  for(int i = 0 ; i<2 ; ){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (ledState == 0) {
      ledState = 1;
    } else {
      ledState = 0;}
      i++;
    digitalWrite(ledPin, ledState);}}
  }  

  void get_ir_data(){
if (IrReceiver.decode()){
        irkey = String(IrReceiver.decodedIRData.decodedRawData , HEX); 
       //Serial.println(irkey);
        //Serial.println(results.value, HEX);
        IrReceiver.resume();
  } 
  if(irkey == irCode[0][0]){
    irkey = "";
    if(startTimer == 1){
      resume_tone(27);
    }
    else{
      pause_tone(27);
    }
   }
   else if(irkey == irCode[1][0]){
     irkey = "";
     pause_tone(27);
     //startTimer = 2;
     pressStar=1;
     counter_var=2;
   }
   else if(irkey == irCode[2][0]){
     irkey = "";
     resume_tone(27);
    // startTimer = 1;
    pressStar=0;
    counter_var=1;
   }

  }

void cf_data(){
      HTTPClient httpc;
      // Send HTTP GET request
      httpc.useHTTP10(true);
      httpc.begin(serverName);
      int httpResponseCode = httpc.GET();
     // Serial.println(httpResponseCode);
      if (httpResponseCode>0) {
        digitalWrite(19,HIGH);
        //Serial.println("cf started 4");
        //Serial.print("HTTP Response code: ");
        //Serial.println(httpResponseCode);
        StaticJsonDocument<512> filter;
        filter["result"][0]["phase"] = true;
        filter["result"][0]["name"] = true;
        filter["result"][0]["startTimeSeconds"] = true;
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, httpc.getStream() , DeserializationOption::Filter(filter));
        int doc_item = 0;
        int array_item = 0;
        for(int i=0 ; i<9 ; i++){
        String s = doc["result"][doc_item]["name"].as<String>();
        s = s.substring( int(s.length()) - 7, int(s.length())-1);
        if(s == "Div. 2" || s == "Div. 3" || s == "Div. 4"){
        String s = doc["result"][doc_item]["name"].as<String>();
        s = s.substring( int(s.length()) - 7, int(s.length())-1);
        ContestDiv[array_item] = s;
        ContestPhase[array_item] = doc["result"][doc_item]["phase"].as<String>();
        ContestTime[array_item] = doc["result"][doc_item]["startTimeSeconds"].as<int>();
        doc_item++;
        array_item++;
        }
        else{
          doc_item++; 
        }
        if(doc_item == 8 || array_item == 8){
         break;
        }
        }
        array_item_uni = array_item;
        String tempdiv;
        int temptime;
        String tempphase;
       for(int i = 0; i<array_item/2; i++){
        tempdiv = ContestDiv[i];
        ContestDiv[i] = ContestDiv[array_item-i-1];
        ContestDiv[array_item-i-1] = tempdiv;
        temptime = ContestTime[i];
        ContestTime[i] = ContestTime[array_item-i-1];
        ContestTime[array_item-i-1] = temptime;
        tempphase = ContestPhase[i];
        ContestPhase[i] = ContestPhase[array_item-i-1];
        ContestPhase[array_item-i-1] = tempphase;
    }
      
        }
      // Free resources
      httpc.end();
      digitalWrite(19,LOW);
      }

void main_screen(int chosen,int hor,int minu,int sec_var,char key,String new_hor,String new_minu,String new_sec){
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
 formattedDate = timeClient.getFormattedDate();
 int splitT = formattedDate.indexOf("T");
 dayStamp = formattedDate.substring(5, splitT);
 hours=timeClient.getHours();
 day_var = daysOfTheWeek[timeClient.getDay()];
 minutes = timeClient.getMinutes();
 seconds = timeClient.getSeconds();
 year_var = formattedDate.substring(0,4);
 if(hours>12){
  hours-=12;
  ampm = "Pm";}
 else{
 ampm = "Am";
 }
 if(year_var == "1970"){
  hours = minutes = seconds = 0;
  day_var = "???";
  dayStamp = "0-0";
  }

 int clearr = millis();
 
 
 
 display.setCursor(17,0+15);
 display.setTextSize(2);
 char bufffer[40];
 sprintf(bufffer, "%s:%s:%s",new_hor,new_minu,new_sec);

if (key=='B' and pressB==0) {
  start_hour = hours;
  start_minute = minutes;
  start_second = seconds;
  counter_var=1;
  pressB=1;
  startTimer = 1;
  key = '1';
  pressed+=1;
}
else if(key=='B' and pressB==1){
display.clearDisplay();
display.display();
pressB=0;
counter_var=0;
key = '1';
}

if(key=='*' and pressStar==0){
display.clearDisplay();
display.display();  
pressStar=1;
counter_var=2;
key = '1';
  }
else if(key=='*' and pressStar==1){
  pressStar=0;
  counter_var=1;
  }  
if(counter_var==0){
  if(pressed >= 0 and chosen!=-1){
  display.clearDisplay();
  display.print("Saving..");
  display.display();
  if ((millis() - lastTime) > timerDelay){
    char datum[200];
    sprintf(datum, "%s/%s/%i:%i:%i/%i:%i:%i/%i:%i:%i",str[square][chosen],dayStamp,start_hour,start_minute,start_second,new_hor,new_minu,new_sec,hours,minutes,seconds);
    if(WiFi.status() != WL_CONNECTION_LOST && WiFi.status()== WL_CONNECTED){
    int x = ThingSpeak.writeField(myChannelNumber, 1,datum, myWriteAPIKey);
    
    if(x == 200){
       display.println("saved!"); 
       //delay(500);
       }
    else{
      
    }}
    else{
      saving+=1;
      char datum[200];
      sprintf(datum, "%s/%s/%i:%i:%i/%i:%i:%i/%i:%i:%i",str[square][chosen],dayStamp,start_hour,start_minute,start_second,new_hor,new_minu,new_sec,hours,minutes,seconds);
      saved_data[saving]=datum;
      display.print("saved to var");
      //delay(500);
      }}
      lastTime = millis();
     pressed = -1;
      }
  else{
  display.print("00:00:00");}
  startTimer = 0;

}
else if(counter_var==1){
  display.print(bufffer);
  startTimer = 1;
}

else if(counter_var==2){
  display.print(bufffer);
  startTimer = 2;
}

 display.setTextSize(1);
 display.setCursor(0,20+15);
 char buffer[40];
 sprintf(buffer,"%i:%i:%i %s %s %s",hours,minutes,seconds,ampm,day_var,dayStamp);
 display.print(buffer);
 int savings = saving;
 if(saving>-1 and WiFi.status() != WL_CONNECTION_LOST && WiFi.status()== WL_CONNECTED){
  display.print("saving");
      for(int i=0;i<savings +1;){
        if((millis() - lastTime ) > timerDelay){
      int x = ThingSpeak.writeField(myChannelNumber,1,saved_data[i], myWriteAPIKey);
      if(x==200){
       i++;
       saving -=1;
       }
       else{
        }
       lastTime = millis();
       }
      }
      }
 if(WiFi.status() != WL_CONNECTION_LOST && WiFi.status()== WL_CONNECTED){ 
  if(lastUpdate+1200000<millis()){
 timeClient.update();
 cf_data();
 lastUpdate = millis();
 }
 else if(millis()<60000 && ntp_updated == 0){
  timeClient.update();
  if(year_var != "1970"){
  ntp_updated = 1 ;}
  }

  if(millis() > 60000 && cf_updated == 0 && cf_tries < 10){
  cf_data();
  if(ContestDiv[0] != "Div -1"){
  cf_updated = 1;}
  cf_tries++;
  }

  }
  }




void cf_div_screen(String upcoming,int contest_date,String ratingAndProblems ,int hor , int minu , int sec_var){

char buff_date[32];
sprintf(buff_date, "%02d/%02d %02d:%02d", day(contest_date), month(contest_date) , hour(contest_date) ,minute(contest_date));//, year(t), hour(t), minute(t), second(t));
char buff_time[32];
if(contest_date!=-1){
int cf_relative = contest_date - timeClient.getEpochTime();
int days_cf = (cf_relative/86400) ;
int hours_cf = (cf_relative - (days_cf*86400)) / 3600 ;
if(hours_cf > 12){
  hours_cf = hours_cf - 12;
}
int minutes_cf = (cf_relative - (days_cf*86400) - (hours_cf*3600))/60 ;
//int seconds_cf = (cf_relative - (days_cf*86400) - (hours_cf*3600) - (minutes_cf*60))/60 ;
if(days_cf<99 && days_cf > 0){
sprintf(buff_time, "%02d - %02d:%02d", days_cf, hours_cf ,minutes_cf );}
else{
sprintf(buff_time, "00 - 00:00");
}
}
else{
  
sprintf(buff_time, "00 - 00:00");
}

display.setCursor(30,1);
display.setTextSize(2);
display.print(upcoming);
display.setTextSize(1);
display.setCursor(25,20);
display.print(buff_date);
display.setTextSize(2);
display.setCursor(0,35);
display.print(buff_time);
display.setTextSize(1);
display.setCursor(0,56);
display.print("rating: 837 || x left");
}  






void IRAM_ATTR onTimer(){
  if(startTimer == 1){
  sec_var+=1;
  if(sec_var>=59){
    sec_var=0;
    minu++;
    }
  if(minu>=59){
    minu=0;
    hor++;
    } }
   else if(startTimer ==0){
   sec_var = 0;
   hor =0;
   minu = 0;
   } 
   beautiful_int(sec_var,hor,minu);
 } 



















void setup() {
    Serial.begin(9600);
    
if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
   
  }
  int connection_begin = millis();
     if(WiFi.status() != WL_CONNECTED){
      while(WiFi.status() != WL_CONNECTED && millis() <= connection_begin+20000){
        WiFi.begin(ssid, password);  
        break;    
      } 
     
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
  web_server();
  pinMode(19,OUTPUT); 
  pinMode(27,OUTPUT);  
  //IrReceiver.enableIRIn();
  
  IrReceiver.begin(RECV_PIN , ENABLE_LED_FEEDBACK);
  //IrReceiver.blink13(true);
}
void loop() { 
char key = keypad.getKey();  
if(key){
  display.ssd1306_command(SSD1306_DISPLAYON);
  lastPress = millis();
  }  
if(lastPress+30000<millis()){
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  } 
if(key == 'C'){
  if(pressD != 1){
  if(square==activities-1){
      square = 0;}
  else{
    square+=1;
    }}
    if(pressA == 1){
     display.clearDisplay();
    print_label(square);
    pressA = 1;
    key = '?';
    }
    if(pressD == 1){
    if(cf_square== array_item_uni-1){
      cf_square = 0;
    }
    else{
      cf_square +=1;
    }
     display.clearDisplay();
    cf_div_screen(ContestDiv[cf_square],ContestTime[cf_square],"rating: 837 || 5 left" , hor , minu , sec_var);
    display.display();
    pressD = 1;
    
    }
    
    }   
if(key=='A' and pressA!=1 and pressD!=1) {
  //int lastSquare = square;
  display.clearDisplay();
  print_label(square);
  pressA=1;
  key = '?';
}
else if(key=='A' and pressA==1){
display.clearDisplay();
display.display();
pressA=0;}
if(pressA==1){
edit_key(key);
if(key_num != -48){
    chosen = key_num-1;
  }} 
 if(key=='D' and pressD!=1 and pressA!=1) {
 display.clearDisplay();
cf_div_screen(ContestDiv[cf_square],ContestTime[cf_square],"rating: 837 || 5 left" , hor , minu , sec_var);
display.display();
if(WiFi.status() != WL_CONNECTION_LOST && WiFi.status()== WL_CONNECTED){
 if(cf_updated == 0){
  cf_data();
  cf_updated = 1;
  }}
pressD=1;
 } 
else if(key=='D' and pressD==1){
display.clearDisplay();
display.display();
pressD=0;

} 
if(pressA!=1 and pressD!=1){
    main_screen(chosen,hor,minu,sec_var,key,new_hor,new_minu,new_sec);
   drawer();  
 }
get_ir_data();
if(millis() > last_led +3000 || irkey == "f50a4040"){
int ldr_value = analogRead(34);
last_led=millis();
if((ldr_value<700 && chosen_var() != "sleep") || irkey == "f50a4040"){
  led_begin = millis();
  digitalWrite(19,HIGH);
}
if(millis() - 7000 > led_begin || ldr_value>1000){
  if(irkey != "f50a4040"){
digitalWrite(19,LOW);}

}

}

display.display();
}
