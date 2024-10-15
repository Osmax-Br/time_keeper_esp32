#include <Arduino.h>
#include "ESPAsyncWebServer.h"
#include <press.h> // for multiple buttons regestration
//#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
//#include <WiFiUdp.h>
//#include <NTPClient.h>
//#include <SPI.h>
#include <Wire.h>
//#include <WiFiUdp.h>
#include "html.h" // the website code
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Time.h> // for using the internal rtc
#include <IRremote.hpp>
//#include "DHT.h"
#include <Preferences.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
//******************************
// dht int
#define DHTPIN 13 // dht11 pin
#define DHTTYPE DHT11
//DHT dht(DHTPIN, DHTTYPE);
//int last_temp_update = 0; // dht11 updates between 2 seconds intreval
//float temperature,humidity,heat_index = 0;  // storing heat informations



//bme280 
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;
int last_temp_update = 0; // bme280 updates between 2 seconds intreval
float temperature,humidity,pressure,heat_index = 0;  // storing heat informations





/*
INSERT INTO activities (chosen_activity,hours_passed,minutes_passed,seconds_passed,activity_date_month,activity_date_day_number,activity_date_weekday,activity_date_hour,activity_date_minute)
      VALUES ('Paul', 32, 32, 32 , 1 ,1 , "wed" ,1 ,1 );
*/
//**********************************************

//oled screen init
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
#include <Fonts/FreeMonoBoldOblique9pt7b.h> //fonts
#include <Fonts/FreeMono9pt7b.h>
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int last_time_screen_on = 0;
bool time_critical = false ; // for not turning the screen off while uploading
int screen_off_seconds = 0;
int screen_off_minutes = 0;
int saves_screen_off_time = 0;
bool time_partitioned = false;

//************************************************
//ui vars
String chosen_value = "Nothing"; //the chosen value from the grid
int select_mode = 0 ; //for maniging selection ways || 0 main screen , 1 grid selection , 2 , 3 screen off , 4 drop down menu , 5 error message
int filled_rect = -1 ; //for inverting text
int table[12][2] = {{0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1}, {2, 1}, {0,2}, {1,2}, {2,2}, {0,3}, {1,3}, {2,3}}; //for storing the coordinats of each square in the grid
const int pages = 4; // ui pages number
int current_page = 0; // for navigation
String str[pages][12] = {{"Anatomy 1","Histology","Bio Chemistry","Arduino","Esp32","Mosque","Code Forces","National culture","psychology","Quran","English","Arabic"},
{"arabic","french","math","physics","clock","science","draw","code forces","python","esp32","book","Gaming"},
{"tidy","fix","souq","/","/","/","famly Mother","famly Father","edit","cook","/","/"},
{"/","/","/","/","/","/","/","/","/","/","/","/"}};
int rect_cord[4][3][4] = {{{0,0,43,18},{42,0,43,18},{84,0,43,18}},{{0,17,43,18},{42,17,43,18},{84,17,43,18}},{{0,33,43,17},{42,33,43,17},{84,33,43,17}},{{0,49,43,15},{42,49,43,15},{84,49,43,15}}};  //the data of each rectangular in the grid eg: width,height,x,y
int lastpress,press_state = 0; //for button class
int cursor[2] = {0,0} ; //the cursor for grid the bottom-left rect is the 0,0 
String error_message_text = "" ;
int last_select_mode = 0; // this is for error message to return to last screen
bool ir_long_press = false; // if we are in ir long press mode
String temp_button_return = ""; // temporary for ir_button function
int last_time_long_press_mode = 0;  // for not regestring multiple ir long presses
bool ok_cancel_button = true ; // left = ok = true (for choosing ok-cancel screens)
//************************************************************
// rtc vars
long gmt_offset = 0 ; 
long gmt_offset_minutes,gmt_offset_hours = 0;
ESP32Time rtc(3600); // utc time offset , here in Syria it is 1 hour = 3600 sec
bool rtc_time_updated = false;  // for not updating the time twice from the internet
int last_rtc_update = 0;    // for not consuming the cpu for updating rtc


//**************************************************************
//network credentials
const char *ssid     = "lemone"; 
const char *password = "Hta87#Mi00";
//const char* ssid = "Najjar";
//const char* password = "tecoof1937";
String ssid_ ,password_ ;
const char* ssid_ap = "esp32";
const char* password_ap = "123456789";
bool ssid_set = true;
bool wifi_set_mode,password_set = false;
int wifi_page = 0;
// for setting specific ip address
IPAddress local_IP(192, 168, 1, 199); //192.168.1.199
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional
int connection_begin = 0; //knowing the last time the device attempted to connect to wifi



//***************************************************************
//buttons vars
//making button clases , each button is a spereate object
button_press button_up(35);
button_press button_down(34);
button_press button_right(33);
button_press button_left(32);
button_press button_selecT(27);
String up,down,right,left,selecT = "" ; // strores button values
int up_bounce,down_bounce,righ_bounce,left_bounce,selecT_bounce = 0;  // stores bounce press values
int last_press_time = 0;   //ignoring multiple presses at the same time for press function
//****************************************************************
// internal rtc timer vars

hw_timer_t * timer = NULL; //setting up the timer
volatile int seconds_passed; // storing the counter seconds in SRAM for faster execution
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile int minutes_passed,hours_passed = 0; //storing the counter values
char time_counter_string[100] ;  // storing the formatted counter time
volatile bool paused = true ; // for pausing timer

//****************************************************************
//last action vars
int before_last_action_time[2],last_action_time[2] = {0,0}; //views last pause/start time
// 0 = hours , 1 = minutes
bool last_action_is_pause = true;
int last_action_register = 0;
//**************************************************************
//storage vars
int data_storage[4][12][3] ;
int data_storage_index = 0;

//****************************************************************
// web server vars
bool server_started = false ; 
WiFiClient  clientt;
AsyncWebServer server(80);
String timer_web_var(){
  return time_counter_string;
}

String chosen_value_web_var(){
  return chosen_value;

}
String pause_web_var(){
  if(wifi_set_mode == false){
  if(paused == true){
    if(data_storage[current_page][data_storage_index][0] == 0 && data_storage[current_page][data_storage_index][1] && data_storage[current_page][data_storage_index][2]){
      return "Start";}
    else{  
    return "Resume";}
  }
  else if(paused == false ){
    return "Pause";
  }}
  else if(wifi_set_mode == true){
if(ssid_set == true && password_set == false){
    return "SSID";
 } 
if(ssid_set == false &&password_set == true){
  return "Password";
 }}
 
}
String var_processor(const String& var){
  if(var == "Timer"){
    return timer_web_var();
  }
  else if(var == "Chosen_server"){
    return chosen_value_web_var();
    }
  else if(var == "Pause"){
    return pause_web_var();
    }  
  return String();
}
//**************************************************************
//multi-tasking
TaskHandle_t ntp_time;  // getting the tine from internet 
TaskHandle_t buzzer_tone;

//************************************************************
// sql vars
//String sql_server_string = "http://192.168.1.11/esp32sql/data_base_script.php"; 
const char* sql_server = "http://192.168.1.9/esp32sql/data_base_script.php"; 



//********************************************************
//drop screen vars

//the name of options
String options_list[4][4] = {{"End day&upload","change Time zone","Edit timer","chose temp unit"},{"ssid & password","IR settings","server's ip","screen turn off"},{"alarm","email","reset activity","test rgb"},{"Reset All","internal temp","buzzer settings","Contanct me"}};
int scroll_bar_place = 1; // for moving the cursor
int options_outer_counter = 0;  // counting the pages
int block_cursor = 0; // conuting the inner values inside page
int options_select_mode = 0 ; // the main var in drop screen , defines the current option selected
// 0 = menu , 1 = end&upload ....etc

int get_date_cursor = 0 ; // 0 right , 1 left
bool got_the_date_from_db = false ; // not being used
String current_get_day_and_month ;  // temp for storing date (not being used)




//******************************************************
//rgb led vars
#define PIN_RED    12 // GPIO23
#define PIN_GREEN  25 // GPIO22
#define PIN_BLUE   15 // GPIO21
//                       red 0      green 1   blue2     yellow 3    cyan 4      maginta 5   pink 6     off 7
int rgb_values[15][3] = {{255,0,0},{0,255,0},{0,0,255},{250,100,3},{7,245,201},{180,7,245},{245,7,75},{0,0,0}};
String rgb_values_name[15] = {"Red","Green","Blue","Yellow","Cyan","Maginta","Pink","Off"}; //this is for test rgb option
int current_rgb = 0;  // this is for test rgb option


//***************************************************
// prefrences vars
Preferences saves;

//***************************************************
//buzzer vars
int buzzerpin = 23;
String buzzer_tone_select = "" ;
int one_time_short_interval = 10 ;
int one_time_long_interval = 25 ;
int error_interval = 100 ;
bool buzzer_always_on = true;
bool buzzer_error_sound = true;
bool buzzer_pause_sound = true;
bool buzzer_press_sound = true;
int buzzer_settings_cursor = 0;


//***************************************************
//7 segment vars
int STCP_pin = 16 ; //latch
int SHCP_pin = 4 ; //clock
int DS_pin = 2 ;
int dec_digits   [11] {1,79,18,6,76,36,32,15,0,4,127};
int seg_parts[4] = {17,5,18,19};  
int next_seg = 0;
int next_seg_time = 0;
int temp_num = 0;
TaskHandle_t segment_handler;



//internal temp sensor vars
#ifdef __cplusplus

extern "C" {

#endif

uint8_t temprature_sens_read();

#ifdef __cplusplus

}

#endif

uint8_t temprature_sens_read();






void segment_driver(int number){
 
  /*
int first_digit = (number/1000)%10 ;                 //,second_digit,third_digit,fourth_digit = 0;
int second_digit = (number / 100)%10 ;
int third_digit = (number / 10)%10 ;
int fourth_digit = number % 10 ;


if(number < 1000){
  first_digit = 10;
  if(number < 100){
    second_digit = 10;
    if(number < 10){
      third_digit = 10;
    }
  }
}

//17 5 18 19
  //digitalWrite(seg_parts[0],1);
  //digitalWrite(seg_parts[1],0);
  //digitalWrite(seg_parts[2],0);
  //digitalWrite(seg_parts[3],0);
REG_WRITE (GPIO_OUT_W1TS_REG, BIT17);
REG_WRITE (GPIO_OUT_W1TC_REG, BIT5);
REG_WRITE (GPIO_OUT_W1TC_REG, BIT18);
REG_WRITE (GPIO_OUT_W1TC_REG, BIT19);
//REG_WRITE (GPIO_OUT_W1TC_REG, BIT16);
  digitalWrite(STCP_pin,LOW);
  
  shiftOut(DS_pin,   SHCP_pin, LSBFIRST,dec_digits[first_digit]);
 
  digitalWrite(STCP_pin, HIGH);
//REG_WRITE (GPIO_OUT_W1TS_REG, BIT16);  
  
  vTaskDelay(5);

  //digitalWrite(seg_parts[0],0);
  //digitalWrite(seg_parts[1],1);
  //digitalWrite(seg_parts[2],0);
  //digitalWrite(seg_parts[3],0);
REG_WRITE (GPIO_OUT_W1TC_REG, BIT17);
REG_WRITE (GPIO_OUT_W1TS_REG, BIT5);
REG_WRITE (GPIO_OUT_W1TC_REG, BIT18);
REG_WRITE (GPIO_OUT_W1TC_REG, BIT19);
//REG_WRITE (GPIO_OUT_W1TC_REG, BIT16);
  digitalWrite(STCP_pin,LOW);
  
  shiftOut(DS_pin,   SHCP_pin, LSBFIRST,dec_digits[second_digit]);
 
  digitalWrite(STCP_pin, HIGH);
//REG_WRITE (GPIO_OUT_W1TS_REG, BIT16);  
  vTaskDelay(5);

 

  //digitalWrite(seg_parts[0],0);
  //digitalWrite(seg_parts[1],0);
  //digitalWrite(seg_parts[2],1);
  //digitalWrite(seg_parts[3],0);

REG_WRITE (GPIO_OUT_W1TC_REG, BIT17);
REG_WRITE (GPIO_OUT_W1TC_REG, BIT5);
REG_WRITE (GPIO_OUT_W1TS_REG, BIT18);
REG_WRITE (GPIO_OUT_W1TC_REG, BIT19);
 
 //REG_WRITE (GPIO_OUT_W1TC_REG, BIT16);
  digitalWrite(STCP_pin,LOW);

  shiftOut(DS_pin,   SHCP_pin, LSBFIRST,dec_digits[third_digit]);
 
  digitalWrite(STCP_pin, HIGH);
  //REG_WRITE (GPIO_OUT_W1TS_REG, BIT16);
  
  vTaskDelay(5);






  //digitalWrite(seg_parts[0],0);
 // digitalWrite(seg_parts[1],0);
 // digitalWrite(seg_parts[2],0);
  //digitalWrite(seg_parts[3],1);
REG_WRITE (GPIO_OUT_W1TC_REG, BIT17);
REG_WRITE (GPIO_OUT_W1TC_REG, BIT5);
REG_WRITE (GPIO_OUT_W1TC_REG, BIT18);
REG_WRITE (GPIO_OUT_W1TS_REG, BIT19);
//REG_WRITE (GPIO_OUT_W1TC_REG, BIT16);
  digitalWrite(STCP_pin,LOW);
  shiftOut(DS_pin,   SHCP_pin, LSBFIRST,dec_digits[fourth_digit]);
  digitalWrite(STCP_pin, HIGH);
//  REG_WRITE (GPIO_OUT_W1TS_REG, BIT16);
  
  vTaskDelay(5);
*/
}

void segment_driver_display(void * parameter){
  for(;;){
     vTaskDelete(nullptr);
    if(buzzer_error_sound == true){
int temp_nem = data_storage[current_page][data_storage_index][1]*100 + data_storage[current_page][data_storage_index][0];
//segment_driver(temp_nem);

}
vTaskDelay(5);
}
}

// should put it once
void rgb_display(int rgb_value_index){  // we did 255 - valie because the led is common annode ==> 0 = on  , 1 = off
    analogWrite(PIN_RED,255 - rgb_values[rgb_value_index][0]);
    analogWrite(PIN_GREEN,255 - rgb_values[rgb_value_index][1]);
    analogWrite(PIN_BLUE,255 - rgb_values[rgb_value_index][2]);

}

void buzzer_tone_function(void * pvParameters){ //buzzer tone
    for(;;) {
//void resume_tone(int buzzerpin){
  if(buzzer_tone_select == "resume" && buzzer_always_on == true && buzzer_pause_sound == true){
  int buzzerState = LOW;             // ledState used to set the LED
  unsigned long previousMillis = 0;
  const long interval = 80;
  for(int i =0 ; i<4 ;){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (buzzerState == LOW) {
      buzzerState = HIGH;
    } else {
      buzzerState = LOW;}
    digitalWrite(buzzerpin, buzzerState);
    i++;}
    }
    buzzer_tone_select = "";
  }
/////////////////////////////////////
//void pause_tone(int buzzerpin){
  else if(buzzer_tone_select == "pause" && buzzer_always_on == true && buzzer_pause_sound == true){
  int buzzerState = 0;             // ledState used to set the LED
  unsigned long previousMillis = 0;
  const long interval = 220;
  for(int i = 0 ; i<2 ; ){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (buzzerState == 0) {
      buzzerState = 1;
    } else {
      buzzerState = 0;}
      i++;
    digitalWrite(buzzerpin, buzzerState);}}
    buzzer_tone_select = "";
}



else if(buzzer_tone_select == "one_time_short" && buzzer_always_on == true && buzzer_press_sound == true){
  int buzzerState = 0;             // ledState used to set the LED
  unsigned long previousMillis = 0;
  const long interval = 10;
  for(int i = 0 ; i<2 ; ){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (buzzerState == 0) {
      buzzerState = 1;
    } else {
      buzzerState = 0;}
      i++;
    digitalWrite(buzzerpin, buzzerState);}}
    buzzer_tone_select = "";
}

else if(buzzer_tone_select == "one_time_long" && buzzer_always_on == true && buzzer_press_sound == true){
  int buzzerState = 0;             // ledState used to set the LED
  unsigned long previousMillis = 0;
  const long interval = 25;
  for(int i = 0 ; i<2 ; ){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (buzzerState == 0) {
      buzzerState = 1;
    } else {
      buzzerState = 0;}
      i++;
    digitalWrite(buzzerpin, buzzerState);}}
    buzzer_tone_select = "";
}
else if(buzzer_tone_select == "error" && buzzer_always_on == true && buzzer_error_sound == true){
  int buzzerState = 0;             // ledState used to set the LED
  unsigned long previousMillis = 0;
  const long interval = 50;
  for(int i = 0 ; i<20 ; ){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (buzzerState == 0) {
      buzzerState = 1;
    } else {
      buzzerState = 0;}
      i++;
    digitalWrite(buzzerpin, buzzerState);}}
    buzzer_tone_select = "";
}





else{}




}} 



// must add last_time var
void post(int page , int activity , int day_of_upload , int month_of_upload , int year_of_upload) { // for pushing the data into the sqlite data base on the pc
// on pc I used wimp to host a server
 // http post
    if(WiFi.status()== WL_CONNECTED){ 
      HTTPClient http;
      
     // String s = sql_server;
      http.begin(sql_server);
      	
      http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header
     
     char post_message[2000] ; //storing the post message                                                                                                                                                                                                                                //rtc.getTime("%I:%M:%S %p %a %d/%m")
     // should change the values in the next line to varaibles
     sprintf(post_message,"chosen_activity=%s&hours_passed=%i&minutes_passed=%i&seconds_passed=%i&activity_date_month=%i&activity_date_day_number=%i&activity_date_weekday=%s&activity_date_hour=%s&activity_date_minute=%s&activity_date_year=%i",str[page][activity],data_storage[page][activity][2],data_storage[page][activity][1],data_storage[page][activity][0],month_of_upload,day_of_upload,rtc.getTime("%a"),rtc.getTime("%H"),rtc.getTime("%M"),year_of_upload);
      int httpResponseCode = http.POST(post_message);   
      if(httpResponseCode>0){
  
    String response = http.getString();  //Get the response to the request
    Serial.println(response);
    }
    Serial.print(httpResponseCode);
    /* else{
          last_select_mode = select_mode;
          select_mode = 5;
          error_message_text = "error during upload";
          options_select_mode = 0;
          last_press_time = millis();
      }*/
      http.end();
    }}

String get(){
  String return_var = "";
      if(WiFi.status() != WL_CONNECTED){
          buzzer_tone_select = "error";
          last_select_mode = select_mode;
          select_mode = 5;
          error_message_text = "connect to wifi";
          options_select_mode = 0;
          last_press_time = millis();

    }
if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      String serverPath = sql_server;
      
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      if (httpResponseCode>0) {
      
        String payload = http.getString();
        return_var = payload ;
      
      }
      else {
        
      }
      // Free resources
      http.end();
    }

  return return_var;

}





void web_server(){
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, var_processor);
  });
  
  server.on("/timer", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", timer_web_var().c_str());
  });
   server.on("/chosen_server", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", chosen_value_web_var().c_str());
  });
    server.on("/Pause", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "ok");
  });
  server.on("/btn", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", pause_web_var().c_str());
  });
  server.on("/Resume", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "ok");
  });
   server.on("/PauseBtn", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(paused==true && chosen_value != "Nothing"){
        paused = false;
        buzzer_tone_select = "resume";
      }
    else if(paused == false){
        paused = true;
        buzzer_tone_select = "pause";
      } 
    
    request->send(200, "text/plain", "ok");
  });
  server.on("/choseee", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage = "";
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if(wifi_set_mode == false){
    if (request->hasParam("input1")) {
      inputMessage = request->getParam("input1")->value();
      inputParam = "input1";
     int chosen_value_index = 0; // for chosing the value index from str array (activity) 
      for(int i =0 ; i<12 ;i++){
      if(str[3][i] == "/"){
        chosen_value_index = i;
        break;
     }}
    // must put semaphore
    current_page = 3 ;
    str[3][chosen_value_index] = inputMessage;
    chosen_value = str[3][chosen_value_index];
      }}
    else if(wifi_set_mode == true ){
        if(ssid_set == true){
    if (request->hasParam("input1")) {
      inputMessage = request->getParam("input1")->value();
      inputParam = "input1";
      ssid_ = inputMessage ;
      ssid_set = false;
      }}
    else if(ssid_set == false){
    if (request->hasParam("input1")) {
      inputMessage = request->getParam("input1")->value();
      inputParam = "input1";
      password_ = inputMessage ;
      password_set = true;
      //ssid_set = true;
      }        



    }}

    request->send(200, "text/html", " <meta http-equiv='refresh' content='0; URL=http://192.168.1.199/'> ");

  });
 
  server.begin();
}



String partition_time(long time){
  long hours = time/3600;
  long minutes = (time - (hours * 3600))/60 ; 
  long seconds = time - (hours*3600 + minutes*60) ;
  char return_value[200];
  sprintf(return_value,"%02i:%02i:%02i",hours,minutes,seconds);
  return return_value;
}




int last_buzzer_time = 0;

// error message function
void error_message(String error_text_message,int last_select_mode){
  
  if(select_mode == 5){
  rgb_display(0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print(error_text_message);
  display.drawRoundRect(40,45,50,15,3,WHITE); 
  display.setCursor(60,48);
  display.print("ok");
  display.display();
  
  
  if(selecT == "pressed" && millis() > last_press_time + 100){
    //error_buzzer_on = false;
    display.clearDisplay();
    select_mode = last_select_mode;
    last_select_mode = 0;
    last_press_time = millis();
    return ;
  }
}}




void ir_button(){//32086590080
    if(IrReceiver.decodedIRData.decodedRawData == 3208659008 || IrReceiver.decodedIRData.decodedRawData == 4061003520){ 
  
  if(ir_long_press == false){
  ir_long_press = true;
  last_time_long_press_mode = millis();
  IrReceiver.decodedIRData.decodedRawData = 0;
  }
  else if(ir_long_press == true){
  ir_long_press = false;
  IrReceiver.decodedIRData.decodedRawData = 0;
  }
  

}




if(ir_long_press == false){
  temp_button_return = "pressed";
  rgb_display(7);
  
}
else{
  temp_button_return = "long_pressed";
  rgb_display(4);
  
}


if(millis() > last_time_long_press_mode + 10000 && ir_long_press == true){
    rgb_display(7);
    last_time_long_press_mode = millis();
    ir_long_press = false;
}



if(IrReceiver.decodedIRData.decodedRawData == 4060954688 || IrReceiver.decodedIRData.decodedRawData == 3927310080){ 
  selecT = temp_button_return;
  if(temp_button_return = "long_pressed"){
    ir_long_press = 0;
  }
  IrReceiver.decodedIRData.decodedRawData = 0;
}
if(IrReceiver.decodedIRData.decodedRawData == 4094378048|| IrReceiver.decodedIRData.decodedRawData == 3208707840){
        up = temp_button_return; 
          if(temp_button_return = "long_pressed"){
            ir_long_press = 0;}
        IrReceiver.decodedIRData.decodedRawData = 0;
}
if(IrReceiver.decodedIRData.decodedRawData == 4044243008 || IrReceiver.decodedIRData.decodedRawData == 3860463360){
        down = temp_button_return;
          if(temp_button_return = "long_pressed"){
          ir_long_press = 0;} 
        IrReceiver.decodedIRData.decodedRawData = 0;
}
if(IrReceiver.decodedIRData.decodedRawData == 3994107968 || IrReceiver.decodedIRData.decodedRawData == 4127850240){
        right = temp_button_return; 
        if(temp_button_return = "long_pressed"){
        ir_long_press = 0;}
        IrReceiver.decodedIRData.decodedRawData = 0;
}
if(IrReceiver.decodedIRData.decodedRawData == 4010819648 || IrReceiver.decodedIRData.decodedRawData == 4161273600){
        left = temp_button_return; 
        if(temp_button_return = "long_pressed"){
        ir_long_press = 0;
  }
        IrReceiver.decodedIRData.decodedRawData = 0;
}
if(IrReceiver.decodedIRData.decodedRawData == 3910549568 || IrReceiver.decodedIRData.decodedRawData == 3125149440){
        paused = true;
        buzzer_tone_select = "pause";
        IrReceiver.decodedIRData.decodedRawData = 0;
}
if(IrReceiver.decodedIRData.decodedRawData == 3893837888 || IrReceiver.decodedIRData.decodedRawData == 3091726080){
  if(chosen_value != "Nothing"){
        paused = false;
        buzzer_tone_select = "resume";}
        IrReceiver.decodedIRData.decodedRawData = 0;
}
if(IrReceiver.decodedIRData.decodedRawData == 3877126208 || IrReceiver.decodedIRData.decodedRawData == 3108437760){
        if(paused == true){
          buzzer_tone_select = "pause";
        }
        else if (paused== false){
          buzzer_tone_select = "resume";
        }
        IrReceiver.decodedIRData.decodedRawData = 0;
}


// red 39105495680
// gre 3893837888
// yellow 38771262080



  if (IrReceiver.decode()) {
    //Serial.println(IrReceiver.decodedIRData.decodedRawData);
      IrReceiver.resume(); // Enable receiving of the next value
  } 

}





void get_ntp_time( void * pvParameters ){ //get time data from ntp
    for(;;) {
    if((WiFi.status() == WL_CONNECTED && rtc_time_updated == false) || (WiFi.status() == WL_CONNECTED && rtc_time_updated == true && millis() > last_rtc_update + 86400)){
    // getting the time from ntp server
  configTime(7200,0, "pool.ntp.org");   // utc offset , daylight saving , ntp server
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)){
    rtc.setTimeStruct(timeinfo); // set the rtc time from ntp
    rtc_time_updated = true ; // dont reconntect to ntp servers
    last_rtc_update = millis();
  }
      

    }
  }
}



void IRAM_ATTR onTimer() {      //Defining Inerrupt function with IRAM_ATTR for faster access
if(paused == false && chosen_value != "Nothing"){
 portENTER_CRITICAL_ISR(&timerMux); // for stopping other functions of changing this value at the same time
 data_storage[current_page][data_storage_index][0] ++ ;
 portEXIT_CRITICAL_ISR(&timerMux);
}}



void connect_wifi(){
  //WiFi.mode(WIFI_AP);
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {  // ensuring no error happened
  }
  display.print("connecting.......");
  display.display();
  connection_begin = millis();  //for timing out
     if(WiFi.status() != WL_CONNECTED){
      while(WiFi.status() != WL_CONNECTED && millis() <= connection_begin+20000){
        WiFi.begin(ssid,password);
        break;    
      } 
     
    }


}

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
  int _x = table[i][0]; // getting the coordintaes of each rectangular
  int _y = table[i][1];  
  if(i==9){
    i2 = -2;  //adjusting y shift
  }
  display.setCursor(4 + 41 * _x,3 + (17 * _y) +i2); //setting writing cursor
  if(_x == filled_rect_x && _y==filled_rect_y){ //&& selecT_bounce == 1){   //this is for inverting the text color while selecting
  display.setTextColor(BLACK);
  display.print(str[current_page][i].substring(0,6));} //printing the labels
  else{
 display.setTextColor(WHITE);
  display.print(str[current_page][i].substring(0,6));
  }
  }
}

void grid_navigiation(){
    select_mode = 1;
    int x = cursor[0]; // getting current cursor coordintes
    int y = cursor[1];
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
    display.clearDisplay(); //without it the lables will over lap
    display.fillRect(rect_cord[y][x][0],rect_cord[y][x][1],rect_cord[y][x][2],rect_cord[y][x][3], WHITE); //making the rectangular white (needed to swap the x&y because of typing error)



if(selecT == "pressed"){ //the action which happens when something is selected
int x = cursor[0]; //the same code used in the function print_label
int y = cursor[1]; //the code is mainly to change from coordinates in cusor to a number which allows us to know the value selected

for(int i=0 ; i<12 ; i++){
  int x1 = table[i][0];
  int y1 = table[i][1];
  if(x1==x && y==y1){
    
    chosen_value = str[current_page][i]; //the Actual selected value 
    data_storage_index = i ; //for knowing wher to store time data
    select_mode = 0;
    break;
  }
}

}

if(right == "long_pressed" && current_page != 3){ //for flipping pages and ensuring that an error doesnt happen
  current_page +=1 ;   // change page
}
if(left == "long_pressed" && current_page != 0){
  current_page -=1 ;   // change page 
}

print_label(current_page,cursor[0],cursor[1]); // printing the lables
}



void main_screen(){


 // rgb_display(7);
 display.clearDisplay();
 display.setTextColor(WHITE);
  display.setTextSize(1);
 //display.print("choice: ");
  if(chosen_value=="Nothing"){
    display.setCursor(45,53);
    display.print("Nothing");
    }
  else{
    display.setCursor(64 - chosen_value.length()*3,53);
    display.print(chosen_value);
    } 
  display.setFont(NULL);  
  display.setCursor(17,0+15);
  display.setTextSize(2);
  display.print(time_counter_string);
  display.setTextSize(1);
  display.setCursor(0,20+15);
  if(rtc_time_updated == true){
 display.print(rtc.getTime("%I:%M:%S %p %a %d/%m")); //printing current time from the rtc
  }
  else{
    display.print("Error! cant get time"); 
  }
 



 if(selecT == "pressed" && chosen_value=="Nothing" && millis() > last_press_time + 100){
    last_select_mode = select_mode;
    select_mode = 5;
    buzzer_tone_select = "error";
    error_message_text = "choose something";
    last_press_time = millis();
  }

  display.display();
}


void update_last_action(){ 
/* views when was the last time when pause/start pressed , have a dual action memory , doesnt register an action if the seperation
time is less than a minute
*/
// pause --> resume
if(last_action_register < 1000 && last_action_time[0] == 1 && last_action_time[1] == 0 && millis() < 10000){
  last_action_time[0] = 0; //fixing a problem where on startup shows 1:00
}
  if(last_action_is_pause==true && paused == true){
    if(last_action_register + 120000 < millis() || last_action_register < 1000){ //doesnt register actions less that 2 min apart 
    before_last_action_time[0] = last_action_time[0]; //dual memory saving
    before_last_action_time[1] = last_action_time[1];
    last_action_register = millis();
    }
    last_action_time[0] = rtc.getTime("%I").toInt();  //updating
    last_action_time[1] = rtc.getTime("%M").toInt();

    last_action_is_pause = false; 
    
  }
  else if(last_action_is_pause == false && paused == false){
    if(last_action_register + 120000 < millis() || last_action_register < 1000){
    before_last_action_time[0] = last_action_time[0];
    before_last_action_time[1] = last_action_time[1];
    last_action_register = millis();
    }
    last_action_time[0] = rtc.getTime("%I").toInt();
    last_action_time[1] = rtc.getTime("%M").toInt();
    last_action_is_pause = true;
    
  }
}

void screen_off(){
 
 if((right=="pressed" || left == "pressed" || up == "pressed" || down == "pressed" || selecT == "pressed")){  // wake the screen up
  //for not interfering with the grid selection
  display.ssd1306_command(SSD1306_DISPLAYON);
  if(select_mode == 3){
  select_mode = 0;    // back to main screen
  }
  last_time_screen_on = millis();
  }  
if(last_time_screen_on+saves_screen_off_time<millis() && time_critical == false){  // change the auto display_off time 
  select_mode = 3; // that means the screen is off
  display.ssd1306_command(SSD1306_DISPLAYOFF); // switch the display off
  options_select_mode = 0;
  }  
}


void counter_formatting(){
  
if(data_storage[current_page][data_storage_index][0] > 59){
  portENTER_CRITICAL(&timerMux);
  data_storage[current_page][data_storage_index][0] = 0;
  portEXIT_CRITICAL(&timerMux);
  data_storage[current_page][data_storage_index][1] ++ ;
}
if(data_storage[current_page][data_storage_index][1] == 59){
  data_storage[current_page][data_storage_index][1] = 0;
  data_storage[current_page][data_storage_index][2] ++ ;
}
sprintf(time_counter_string,"%02i:%02i:%02i",data_storage[current_page][data_storage_index][2],data_storage[current_page][data_storage_index][1],data_storage[current_page][data_storage_index][0]);

}

void last_action_screen(){
    display.clearDisplay();
    display.setCursor(33, 8);
    display.drawRoundRect(10, 0, 105, 35, 5, 1);
    display.setTextSize(2);
    display.printf("%02i:%02i",last_action_time[0],last_action_time[1]);
    display.setTextSize(1);
    display.setCursor(15, 25);
    if(paused == true){
    display.print("paused");}
    else{
      display.print("resume");
    }
    display.drawRoundRect(17, 40, 90, 24, 5, 1);
    display.setCursor(50, 45);
    display.printf("%02i:%02i",before_last_action_time[0],before_last_action_time[1]);
    display.setCursor(20, 52);
    if(last_action_is_pause == true){
    display.print("pause");}
    else{
      display.print("resume");
    }
    display.display();
}



void temp_screen(){
  display.clearDisplay();

  if(millis() > last_temp_update + 2000){
    //dht 11 version (depricated)
    /*
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    heat_index = dht.computeHeatIndex(temperature, humidity, false);
    last_temp_update = millis();*/

    //bme280 version
    humidity = bme.readHumidity();
    temperature = bme.readTemperature();
    pressure = bme.readPressure() / 100.0F ;
    last_temp_update = millis();
}
if(temperature > -30 && temperature < 100){
    display.setCursor(0,0);
    display.setTextSize(1);
    display.print("Temp  ");
    display.setTextSize(2);
    display.printf("%.01f C%c",temperature,(char)247);
    display.setCursor(0,25);
    display.setTextSize(1);
    display.print("Humid  ");
    display.setTextSize(2);
    display.printf("%.01f %%",humidity);
    display.setCursor(0,50);
    display.setTextSize(1);
    display.print("Press ");
    display.setTextSize(2);
    display.printf("%.01f hPa",pressure);
    }
else{
    display.setCursor(0,0);
    display.setTextSize(1);
    display.print("TEMP  ");
    display.setTextSize(2);
    display.print("Error");
    display.setCursor(0,25);
    display.setTextSize(1);
    display.print("HUMID  ");
    display.setTextSize(2);
    display.print("Error");
    display.setCursor(0,50);
    display.setTextSize(1);
    display.print("INDEX ");
    display.setTextSize(2);
    display.print("Error");


} 
if(ir_long_press == false){ 
    if(temperature < 20 || temperature > 35){
        rgb_display(0);
    }
    else{
        rgb_display(1);
    }}
    display.display();
}





String buzzer_on_off(bool state){
  if(state == true){
    return "On";
  }
  else if(state == false){
    return "Off";
  }

}


void buzzer_change_state(bool state,const char* key){
  if(state == true){
    //saves.putBool(key,false);
    
    state = false;
  }
  else if(state == false){
  //saves.putBool(key,true);
    state = true;
  }
}

//*******************************************************

void gmt_settings(){
if(time_partitioned == false){
    String gmt_offset_string = partition_time(gmt_offset);
    gmt_offset_minutes = gmt_offset_string.substring(3,5).toInt();
    gmt_offset_hours = gmt_offset_string.substring(0,2).toInt();
    time_partitioned = true;
    }
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setFont(NULL);
    display.setCursor(0, 0);
    display.println("     GMT offset");
    display.setCursor(20, 15);
    display.setTextSize(3);
    display.printf("%02i:%02i",gmt_offset_hours,gmt_offset_minutes);
    if(up == "pressed"){
        if(ok_cancel_button == false && gmt_offset_hours < 12){  //change minutes
            gmt_offset_hours ++ ;
        }
        if(ok_cancel_button == true && gmt_offset_minutes <= 59){  //change minutes
            gmt_offset_minutes ++ ;
        }
        if(gmt_offset_minutes == 60){
          gmt_offset_minutes = 0;
          if(gmt_offset_hours < 12){
         gmt_offset_hours ++ ;}
        }
    }

    if(down == "pressed"){
        if(ok_cancel_button == true && gmt_offset_minutes > 0){  
            gmt_offset_minutes -- ;
        }
        if(ok_cancel_button == false && gmt_offset_hours > -12){  
            gmt_offset_hours -- ;
        }

    }    

    if(ok_cancel_button == false){
    display.drawLine(25, 40, 50, 40, 1);
    }
    else{
    display.drawLine(75, 40, 100, 40, 1);
    }
    display.setTextSize(1);
    display.drawRoundRect(42, 48, 40, 15, 5, 1);
    display.setCursor(57, 51);
    display.print("ok");
    display.display();
    
    
    
    if(left == "pressed" && ok_cancel_button == true){
      ok_cancel_button = false;
    }
    if(right == "pressed" && ok_cancel_button == false){
      ok_cancel_button = true;
    }

    if(selecT == "pressed" && millis() > last_press_time + 100){
        time_partitioned = false;
        gmt_offset = gmt_offset_hours*3600 + gmt_offset_minutes*60 ;
        rtc.offset = gmt_offset ;
        saves.putLong("gmt_offset",gmt_offset);
        last_press_time = millis();
        options_select_mode = 0;
    }
}

//*******************************************************************************************

void buzzer_settings(){
    if(up == "pressed" && buzzer_settings_cursor != 0){
    buzzer_settings_cursor --;
  }
  if(down == "pressed" && buzzer_settings_cursor != 3){
    buzzer_settings_cursor ++;
  }






    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setFont(NULL);
    display.setCursor(0, 2);
    display.println("Buzzer always : ");



    if(buzzer_settings_cursor == 0){
    display.fillRoundRect(90, 0, 38, 12, 3, WHITE);
    display.setCursor(100, 2);
    display.setTextColor(BLACK);
    display.print(buzzer_on_off(buzzer_always_on));}
    else{
    display.drawRoundRect(90, 0, 38, 12, 3, WHITE);
    display.setCursor(100, 2);
    display.setTextColor(WHITE);
    display.print(buzzer_on_off(buzzer_always_on));
    }



    display.setTextColor(WHITE);
    display.setCursor(0, 17);
    display.print("error sound   :");




    if(buzzer_settings_cursor == 1){
    display.fillRoundRect(90, 15, 38, 12, 3, 1);
    display.setCursor(100, 17);
    display.setTextColor(BLACK);
    display.print(buzzer_on_off(buzzer_error_sound));}
    else{
    display.drawRoundRect(90, 15, 38, 12, 3, 1);
    display.setCursor(100, 17);
    display.setTextColor(WHITE);
    display.print(buzzer_on_off(buzzer_error_sound));
    }




    display.setTextColor(WHITE);
    display.setCursor(0, 32);
    display.print("pause/resume  :");


    if(buzzer_settings_cursor == 2){
    display.fillRoundRect(90, 30, 38, 12, 3, 1);
    display.setCursor(100, 32);
    display.setTextColor(BLACK);
    display.print(buzzer_on_off(buzzer_pause_sound));
    }
    else{
    display.drawRoundRect(90, 30, 38, 12, 3, 1);
    display.setCursor(100, 32);
    display.setTextColor(WHITE);
    display.print(buzzer_on_off(buzzer_pause_sound));
    }
    





    display.setTextColor(WHITE);
    display.setCursor(0, 47);
    display.print("press sound   :");



if(buzzer_settings_cursor == 3){
    display.fillRoundRect(90, 45, 38, 12, 3, 1);
    display.setCursor(100, 47);
    display.setTextColor(BLACK);
    display.print(buzzer_on_off(buzzer_press_sound));
}
else{
    display.drawRoundRect(90, 45, 38, 12, 3, 1);
    display.setCursor(100, 47);
    display.setTextColor(WHITE);
    display.print(buzzer_on_off(buzzer_press_sound));
}
    
if(left == "pressed" || right == "pressed"){
    if(buzzer_settings_cursor == 0){
       if(buzzer_always_on == true){
        buzzer_always_on = false;
        saves.putBool("b_always_on",false);
        }
        else if(buzzer_always_on == false){
          buzzer_always_on = true;
          saves.putBool("b_always_on",true);
        }
    }
    else if(buzzer_settings_cursor == 1){
       // buzzer_change_state(buzzer_error_sound,"b_error_sound");
       if(buzzer_error_sound == false){
        buzzer_error_sound = true;
       }
       else if(buzzer_error_sound == true){
        buzzer_error_sound = false;
       }
         saves.putBool("b_error_sound",buzzer_error_sound);
    }
    else if(buzzer_settings_cursor == 2){
       // buzzer_change_state(buzzer_pause_sound,"b_pause_sound");
        if(buzzer_pause_sound == false){
        buzzer_pause_sound = true;
       }
       else if(buzzer_pause_sound == true){
        buzzer_pause_sound = false;
       }
         saves.putBool("b_pause_sound",buzzer_pause_sound);
    }   
    else if(buzzer_settings_cursor == 3){
      if(buzzer_press_sound == false){
        buzzer_press_sound = true;
       }
       else if(buzzer_press_sound == true){
        buzzer_press_sound = false;
       }
       // buzzer_change_state(buzzer_press_sound,"b_press_sound");
        saves.putBool("b_press_sound",buzzer_press_sound);
    }       



}    

    display.display();

}


//**********************************************************

void rgb_settings(){
    display.clearDisplay();
  if(select_mode == 4 && right == "pressed" && options_select_mode == 12 && current_rgb != 7){
   current_rgb ++;
}
if(select_mode == 4 && options_select_mode == 12 && left == "pressed" && current_rgb != 0){
   current_rgb --;
}
  rgb_display(current_rgb);
  display.setCursor(30,10);
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.print(rgb_values_name[current_rgb]);
  display.setCursor(15,40);
  display.setTextSize(1);
  display.printf("(%i ,  %i ,  %i)",rgb_values[current_rgb][0],rgb_values[current_rgb][1],rgb_values[current_rgb][2]);
  display.display();
}


//**************************************************



void select_and_upload_settings(){

  if(rtc_time_updated == false && millis() > last_press_time + 100){
    last_select_mode = select_mode;
    select_mode = 5;
    buzzer_tone_select = "error";
    error_message_text = "please update time";
    options_select_mode = 0;
    last_press_time = millis();
  }
    int day_of_upload = 1;
    int day_of_upload_1 = rtc.getTime("%d").toInt(); //the current real-time day (stored in a var to reduce cpu cycles)
    int day_of_upload_2 = rtc.getTime("%d").toInt() - 1 ; //the other day of upload
    int month_of_upload = rtc.getTime("%m").toInt(); //the current real-time month (stored in a var to reduce cpu cycles)
    int year_of_upload = rtc.getTime("%Y").toInt();
    display.setTextColor(WHITE,BLACK);  //for not transperant bg
    bool done_uploading = false;  // flag to leave the upload menu
    time_critical = true ;  // flag to not turn off the screen while uploading
    //this is for calculating the previous day when the day is 1 (1 - 1 = 0 that's wrong)
    if(day_of_upload_1 == 1){
      if(month_of_upload == 5 || month_of_upload == 7 || month_of_upload == 10 || month_of_upload == 12){
        day_of_upload_2 = 30 ;  //months with 31 days
        month_of_upload -- ;  // change month
      }
      else if(month_of_upload == 3){  //february
        if( ((rtc.getTime("%Y").toInt()%4 == 0) && (rtc.getTime("%Y").toInt() % 100 != 0) ) || rtc.getTime("%Y").toInt() % 400 == 0){ //test if the year is a leap year
          day_of_upload_2 = 29; // leap
          month_of_upload -- ;  // change month
        }
        else{
          day_of_upload_2 = 28; //normal
          month_of_upload -- ;  // change month
        }
      }
      else{
        day_of_upload_2 = 31 ;  //months that are 30 days
        if(month_of_upload == 1){
          year_of_upload -- ;
          month_of_upload = 12;
        }
        else{
        month_of_upload -- ;  // change month 
        }
      }
    }





   //*********************
    bool chose_date = false;    // for navigating between the choosing menu and the actual upload menu
    if(chose_date == false){    // ****the choosing menu****
      display.clearDisplay();
      //not useful
     //  int current_get_day_of_month = current_get_day_and_month.substring(0,current_get_day_and_month.indexOf(",")).toInt();   
      // int current_get_month = current_get_day_and_month.substring(current_get_day_and_month.indexOf(",")+1,current_get_day_and_month.length()).toInt();
      //////////////////
      display.setCursor(5,0);
      display.setTextSize(2);
      display.print("Select Day");
      display.setCursor(35,20);
      display.setTextSize(1);
      display.print("of upload");
      //right set

      display.setTextSize(2);
      display.setCursor(5,40);
      display.print(rtc.getTime("%d")); //current day from rtc
      display.setTextSize(1);
      display.setCursor(30,45);
      display.print("/");
      display.print(rtc.getTime("%02m"));


      display.setTextSize(2);
      display.setCursor(75,40);
      char temp_day_char_storage[30]; // just temp var for formatting
      sprintf(temp_day_char_storage,"%02i" ,day_of_upload_2); // format the printing message
      display.print(temp_day_char_storage);
      display.setTextSize(1);
      display.setCursor(100,45);
      display.print("/");
      char temp_month_print[30];
      sprintf(temp_month_print,"%02i",month_of_upload);
      display.print(temp_month_print);
      if(right == "pressed" && get_date_cursor == 0){ //change the chosen date
      get_date_cursor = 1;
      }
      else if(left == "pressed" && get_date_cursor == 1){
         get_date_cursor = 0;
     }

      if(get_date_cursor == 0){ // draw the line accordingly
         display.drawLine(10,60,40,60,WHITE);
      }
      else if(get_date_cursor == 1){
        display.drawLine(80,60,110,60,WHITE);
      }
      
        if(get_date_cursor == 0){ // set the day of upload
          day_of_upload = day_of_upload_1;  //right (rtc day)
        }
        else if(get_date_cursor == 1){
          day_of_upload = day_of_upload_2; //left (rtc day -1)
        }
        
      if(selecT == "pressed" && millis() > last_press_time + 100 ){ //switch to uplaod screen

          last_press_time = millis();
          chose_date = true ; //choosed the date
      }

      /////////////////
      
    }
// **** uploading screen ****
    if(chose_date == true){ //if the date is chosen from the last screen
    display.clearDisplay();
    display.setCursor(5,0);
    display.setTextSize(2);
    display.print("END OF DAY");
    display.setTextSize(1);
    display.setCursor(0,20);
    display.print("Server: ");
    display.print(String(sql_server).substring(7,19)); //the ip address of server
     display.setCursor(20,35);
     display.print("the day : ");
     char temp_date_print[30];
     sprintf(temp_date_print,"%02i/%02i",day_of_upload,month_of_upload);
     display.print(temp_date_print);  // the chosen date of upload (from last screen)
     int activity_print_counter = 0;  // for printing the progress bar on screen
    rgb_display(5);
    paused = true;
    buzzer_tone_select = "pause";
    for(int i = 0; i<4 ; i++){  // 4 pages
      for(int j =0 ; j<12 ; j++){ // 12 activities
           if(data_storage[i][j][2] != 0 || data_storage[i][j][1] != 0 ){ 
          post(i,j,day_of_upload,month_of_upload,year_of_upload);      // the actual post function
          display.setCursor(20,55);
         char uplaod_progress_message[20] = ""; // dont know why !
         display.print("uploaded ");
         display.print(activity_print_counter); // progress bar
         display.print("/52");
          display.print(uplaod_progress_message);
          display.display();
          activity_print_counter ++ ; // increase the activity counter
           }
      }
    }
    done_uploading = true; // finished uploading (exit screen)
    portENTER_CRITICAL_ISR(&timerMux); // for stopping other functions of changing this value at the same time
    memset(data_storage, 0, sizeof(data_storage));
    portEXIT_CRITICAL_ISR(&timerMux);
    display.display();
    
   if((selecT == "pressed" && millis() > last_press_time + 2000) || done_uploading == true){  // either the process is aborted or it finished
    //got_the_date_from_db = false;
    time_critical = false ; // you can turn the screen off
    options_select_mode = 0;  // go to options menu
    last_press_time = millis();
  }}
  
  
}


//***********************************************************************************
void activity_reset_settings(){

      display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setFont(NULL);
    display.setCursor(0, 0);
    display.println("do you want to reset?");
    display.setCursor(23, 15);
    display.setFont(&FreeMonoBoldOblique9pt7b);
    display.print(chosen_value);
    display.setFont(&FreeMono9pt7b);
    display.setCursor(20, 40);
    display.printf("%02i:%02i:%02i",data_storage[current_page][data_storage_index][2],data_storage[current_page][data_storage_index][1],data_storage[current_page][data_storage_index][0]);
    display.setFont(NULL);
    if(ok_cancel_button == false){
    display.fillRoundRect(20, 45, 40, 15, 3, WHITE);
    display.setCursor(23, 48);
    display.setTextColor(BLACK);
    display.print("Cancel");
    display.setTextColor(WHITE);
    display.drawRoundRect(70, 45, 40, 15, 3, 1);
    display.setCursor(84, 48);
    display.print("ok");}
    else if(ok_cancel_button == true){
    display.drawRoundRect(20, 45, 40, 15, 3, 1);
    display.setCursor(23, 48);
    display.setTextColor(WHITE);
    display.print("Cancel");
    display.fillRoundRect(70, 45, 40, 15, 3,WHITE);
    display.setCursor(84, 48);
    display.setTextColor(BLACK);
    display.print("ok");
    }




    if(left == "pressed" && ok_cancel_button == true){
      ok_cancel_button = false;
    }
    if(right == "pressed" && ok_cancel_button == false){
      ok_cancel_button = true;
    }

    if(ok_cancel_button == false && selecT == "pressed" && millis() > last_press_time + 100){
      ok_cancel_button = true;
      last_press_time = millis();
      options_select_mode = 0;
    }
    else if(ok_cancel_button == true && selecT == "pressed"&& millis() > last_press_time + 100){
      if(chosen_value == "Nothing"){
          last_select_mode = select_mode;
          select_mode = 5;
          error_message_text = "choose an activity";
          buzzer_tone_select = "error";
          options_select_mode = 11;
          last_press_time = millis();
      }
      else{
       portENTER_CRITICAL_ISR(&timerMux); // for stopping other functions of changing this value at the same time
       data_storage[current_page][data_storage_index][0] = 0 ;
       data_storage[current_page][data_storage_index][1] = 0 ;
       data_storage[current_page][data_storage_index][2] = 0 ;
       portEXIT_CRITICAL_ISR(&timerMux);
       paused = true ;
       buzzer_tone_select = "pause";
       last_press_time = millis();
       options_select_mode = 0;
      }

    }


    display.display();
}

//***************************************************************************
void edit_timer(){
  if(paused == false){
    paused = true;
    buzzer_tone_select = "pause";
    }
    display.clearDisplay();
    display.setCursor(64 - chosen_value.length()*3,0);
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setFont(NULL);
    display.print(chosen_value);
    display.setCursor(20, 15);
    display.setTextSize(3);
    display.printf("%02i:%02i",data_storage[current_page][data_storage_index][2],data_storage[current_page][data_storage_index][1]);
    if(up == "pressed"){ // [1] --> minutes || [2] --> hours
        if(ok_cancel_button == false && data_storage[current_page][data_storage_index][2] < 24){  //change hours
            data_storage[current_page][data_storage_index][2] ++ ;
        }
        if(ok_cancel_button == true && data_storage[current_page][data_storage_index][1] <= 59){  //change minutes
            data_storage[current_page][data_storage_index][1] ++ ;
        }
        if(data_storage[current_page][data_storage_index][1] == 60){
          data_storage[current_page][data_storage_index][1] = 0;
          if(data_storage[current_page][data_storage_index][2] < 24){
          data_storage[current_page][data_storage_index][2] ++ ;}
        }
    }

    if(down == "pressed"){
        if(ok_cancel_button == false && data_storage[current_page][data_storage_index][2] > 0){  //change minutes
            data_storage[current_page][data_storage_index][2] -- ;
        }
        if(ok_cancel_button == true && data_storage[current_page][data_storage_index][1] > 0){  //change minutes
            data_storage[current_page][data_storage_index][1] -- ;
        }

    }    

    if(ok_cancel_button == false){
    display.drawLine(25, 40, 50, 40, 1);
    }
    else{
    display.drawLine(75, 40, 100, 40, 1);
    }
    display.setTextSize(1);
    display.drawRoundRect(42, 48, 40, 15, 5, 1);
    display.setCursor(57, 51);
    display.print("ok");
    display.display();
    
    
    
    if(left == "pressed" && ok_cancel_button == true){
      ok_cancel_button = false;
    }
    if(right == "pressed" && ok_cancel_button == false){
      ok_cancel_button = true;
    }

    if(selecT == "pressed" && millis() > last_press_time + 100){
        last_press_time = millis();
        options_select_mode = 0;
    }  
}


//***************************************************************************
void esp32_internal_temp_settings(){
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setFont(NULL);
    display.setCursor(0, 0);
    display.println("Esp 32 internal temp");
    display.setCursor(0, 20);
    display.setTextSize(3);
    display.printf("%.01f C%c",(temprature_sens_read() - 32)/1.8,(char)247);
    display.display();
}

//***********************************************************************************************

void settings_menu_navigation(){

  display.setTextSize(1);
  // rect loop
  for(int i =0 ;i<=4 ; i++){
    if(i==block_cursor){  // fill the rect white when cursor passes
      display.fillRect(0,16*i,110,16,WHITE);
    }
    else{
      display.drawRect(0,16*i,110,16,WHITE);}
      }
  int options_inner_counter = 0;   // internal var only , for inverting text color
  //text loop
  for(int i=5;i<=50;i+=15){
     display.setCursor(9,i+1);
     if(options_inner_counter == block_cursor){ //when cursor passes make rect text black
      display.setTextColor(BLACK);
     }
     else{
      display.setTextColor(WHITE);  // inverting text
     }
    display.print(options_list[options_outer_counter][options_inner_counter]);  // print option labels
    options_inner_counter++;  // print next label inside the same page
    }
  
  
  for(int i=0 ; i <=64 ; i+=8){   // the dots
    display.fillRect(122,i,2,2,WHITE);
          }
// move up
  if(up == "pressed"){
    if(options_outer_counter + block_cursor != 0){  //for not going outside the screen
    block_cursor --;}// move up


    if(block_cursor == -1  && scroll_bar_place !=1){  // change page
      block_cursor = 3; //the last option of the previous page
    scroll_bar_place -= 10; // change the scroll_bar
    options_outer_counter--;} // change page
  }

// move down (same way)
  if(down == "pressed"){
    if(options_outer_counter == 3 && block_cursor ==3){

    }
    else{
    block_cursor ++ ;}
    if(block_cursor == 4  && scroll_bar_place != 31 && options_outer_counter != 4){
      block_cursor = 0;
    scroll_bar_place += 10; 
    options_outer_counter++;}
  }
  display.fillRoundRect(120,scroll_bar_place,5,20,5,WHITE); // print the actual scroll bar
  
 
  if(selecT == "pressed" && options_select_mode == 0){ //entering the first option
    int option_counter_temp = 0;
    option_counter_temp = options_outer_counter*4;
    option_counter_temp += block_cursor ;
    option_counter_temp++ ;
    last_press_time = millis(); // for not regestring multiple presses
    options_select_mode = option_counter_temp;
}}


//**************************************************************************************

void screen_off_settings(){
      if(time_partitioned == false){
    String screen_off_time_string = partition_time(saves_screen_off_time/1000);
    screen_off_seconds = screen_off_time_string.substring(6,8).toInt();
    screen_off_minutes = screen_off_time_string.substring(3,5).toInt();
    time_partitioned = true;
    }
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setFont(NULL);
    display.setCursor(0, 0);
    display.println("   screen off time");
    display.setCursor(20, 15);
    display.setTextSize(3);
    display.printf("%02i:%02i",screen_off_minutes,screen_off_seconds);
    if(up == "pressed"){
        if(ok_cancel_button == false && screen_off_minutes < 3){  //change minutes
            screen_off_minutes ++ ;
        }
        if(ok_cancel_button == true && screen_off_minutes <= 59){  //change minutes
            screen_off_seconds ++ ;
        }
        if(screen_off_seconds == 60){
          screen_off_seconds = 0;
          if(screen_off_minutes < 3){
          screen_off_minutes ++ ;}
        }
    }

    if(down == "pressed"){
        if(ok_cancel_button == false && screen_off_minutes > 0){  //change minutes
            screen_off_minutes -- ;
        }
        if(ok_cancel_button == true && screen_off_seconds > 0){  //change minutes
            screen_off_seconds -- ;
        }

    }    

    if(ok_cancel_button == false){
    display.drawLine(25, 40, 50, 40, 1);
    }
    else{
    display.drawLine(75, 40, 100, 40, 1);
    }
    display.setTextSize(1);
    display.drawRoundRect(42, 48, 40, 15, 5, 1);
    display.setCursor(57, 51);
    display.print("ok");
    display.display();
    
    
    
    if(left == "pressed" && ok_cancel_button == true){
      ok_cancel_button = false;
    }
    if(right == "pressed" && ok_cancel_button == false){
      ok_cancel_button = true;
    }

    if(selecT == "pressed" && millis() > last_press_time + 100){
        saves_screen_off_time = ((screen_off_minutes*60) + screen_off_seconds)*1000;
        if(saves_screen_off_time < 5000){
            saves_screen_off_time = 30000;
            last_select_mode = select_mode;
            select_mode = 5;
            error_message_text = "minimum value is \n      5 seconds !";
            buzzer_tone_select = "error";
            options_select_mode = 0;
            last_press_time = millis();
            time_partitioned = false;
        }
        else if(saves_screen_off_time >= 5000){
        saves.putUInt("screen_off_time",saves_screen_off_time);
        last_press_time = millis();
        time_partitioned = false;
        options_select_mode = 0;}
    }
}

//************************************************************************




void drop_screen(){
  if(options_select_mode == 0){ // main options menu
    settings_menu_navigation();
  } 

// first option menu (end & upload)
  if(options_select_mode == 1){ //first menu + updated date
    select_and_upload_settings();
  }

if(options_select_mode == 12){
 rgb_settings();
}

if(options_select_mode == 3){
  if(chosen_value == "Nothing"){
          last_select_mode = select_mode;
          select_mode = 5;
          error_message_text = "choose an activity";
          buzzer_tone_select = "error";
          options_select_mode = 0;
          last_press_time = millis();
  }
  else{
 edit_timer();
 }
}

if(options_select_mode == 11){
  activity_reset_settings();
}

if(options_select_mode == 2){
    gmt_settings();
}



if(options_select_mode == 8){
  screen_off_settings();
}



if(options_select_mode == 14){
  esp32_internal_temp_settings();
}

if(options_select_mode == 15){
  buzzer_settings();
}


/*
if(options_select_mode == 5){
    wifi_set_mode = true;
    time_critical = true;
    //WiFi.begin("esp32", "123456789");
    if(WiFi.status() == WL_CONNECTED){
      
    }
    if(wifi_page == 0){
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setFont(NULL);
    display.setCursor(0, 0);
    display.print("Set wifi credentials");
    display.setCursor(0, 10);
    display.print("host wifi on phone");
    display.setCursor(0, 25);
    display.print("SSID : esp32");
    display.setCursor(0, 38);
    display.print("Pass : 123456789");

    
    
    
    }
    else if(wifi_page == 1){
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setFont(NULL);
    display.setCursor(0, 0);
    display.println("connect via browser :");
    display.setCursor(0, 15);
    display.print("IP : ");
    display.print(WiFi.localIP());
    display.setCursor(0, 27);
    display.print("Then type ssid and \npress submit ONLY");
    
    }
    else if(wifi_page == 2){
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setFont(NULL);
    display.setCursor(0, 0);
    display.println("wifi saved");
    display.setCursor(0, 15);
    display.setTextSize(1);
    display.println("now you can connect\nto the network and \nconnect to \n   ip:129.168.1.199");
   
    }
    
    if(selecT == "pressed" && wifi_page == 2){
      display.clearDisplay();
    ssid_set = true;
    password_set = false;
    wifi_set_mode = false;

    WiFi.disconnect();
    connect_wifi();
    options_select_mode = 0;
    select_mode = 0;
    } 
        
 if(wifi_page!=2){      
        if(ok_cancel_button == false){
    display.fillRoundRect(20, 48, 40, 15, 3, WHITE);
    display.setCursor(26, 51);
    display.setTextColor(BLACK);
    display.print("Back");
    display.setTextColor(WHITE);
    display.drawRoundRect(70, 48, 40, 15, 3, 1);
    display.setCursor(79, 51);
    display.print("Next");}
    else if(ok_cancel_button == true){
    display.drawRoundRect(20, 48, 40, 15, 3, 1);
    display.setCursor(26, 51);
    display.setTextColor(WHITE);
    display.print("Back");
    display.fillRoundRect(70, 48, 40, 15, 3,WHITE);
    display.setCursor(79, 51);
    display.setTextColor(BLACK);
    display.print("Next");
    }}
    else if(wifi_page == 2){
      display.drawRoundRect(40,50,50,13,1,WHITE); 
      display.setCursor(60,52);
      display.print("ok");
    }




    if(left == "pressed" && ok_cancel_button == true){
      ok_cancel_button = false;
    }
    if(right == "pressed" && ok_cancel_button == false){
      ok_cancel_button = true;
    }

if(selecT == "pressed" && millis() > last_press_time + 100){
  if(ok_cancel_button == true && wifi_page != 1){
      wifi_page ++;
  }
  else if (ok_cancel_button == false && wifi_page != 0){
    wifi_page --;
  }
  last_press_time = millis();
}
    
if(password_set == true){
  wifi_page = 2;
}    

  if(up == "long_pressed" ){
    ssid_set = true;
    password_set = false;
    wifi_set_mode = false;

    WiFi.disconnect();
    connect_wifi();
    options_select_mode = 0;
    select_mode = 0;
  }






  display.display(); 
}*/


if(options_select_mode >= 2 && options_select_mode != 12 && options_select_mode != 11 && options_select_mode != 8 && options_select_mode!= 2 && options_select_mode!= 14  && options_select_mode != 15 && options_select_mode != 3){
    last_select_mode = select_mode;
    select_mode = 5;
    error_message_text = "not done yet";
    buzzer_tone_select = "error";
    options_select_mode = 0;
    last_press_time = millis();
}




}

















void setup() {
    setCpuFrequencyMhz(240);

  sprintf(time_counter_string,"%02i:%02i:%02i",data_storage[current_page][data_storage_index][2],data_storage[current_page][data_storage_index][1],data_storage[current_page][data_storage_index][0]);  // for initilizing the counter string

  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE,BLACK);
  display.setCursor(0,0);
  pinMode(PIN_RED,   OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE,  OUTPUT);
  pinMode(buzzerpin,OUTPUT);
  pinMode(SHCP_pin,OUTPUT);
  pinMode(STCP_pin,OUTPUT);
  pinMode(DS_pin,OUTPUT);
  pinMode(seg_parts[0],OUTPUT);
  pinMode(seg_parts[1],OUTPUT);
  pinMode(seg_parts[2],OUTPUT);
  pinMode(seg_parts[3],OUTPUT);

  timer = timerBegin(0, 80, true);           	// timer 0, prescalar: 80, UP counting
  timerAttachInterrupt(timer, &onTimer, true); 	// Attach interrupt
  timerAlarmWrite(timer,1000000, true);  		// Match value= 1000000 for 1 sec. delay.
  timerAlarmEnable(timer);           			// Enable Timer with interrupt (Alarm Enable)


  //connecting to wifi , the ip address is 192.168.1.199
    connect_wifi();
    display.clearDisplay();

  xTaskCreatePinnedToCore(segment_driver_display,"segment_driver_display",100000,NULL,22,&segment_handler,0); // create a seperate task for getting the ntp time
  xTaskCreatePinnedToCore(get_ntp_time,"ntp_time",10000,NULL,0,&ntp_time,1); // create a seperate task for getting the ntp time
  xTaskCreatePinnedToCore(buzzer_tone_function,"buzzer_tone",10000,NULL,0,&buzzer_tone,1); // create a seperate task for buzzer tones
   if(WiFi.status() == WL_CONNECTED && server_started == false){
   web_server();
   server_started = true ;
   }

   IrReceiver.begin(14, ENABLE_LED_FEEDBACK);
   
  // dht.begin();
    unsigned status;
   status = bme.begin(0x76);
       if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
     //   while (1) delay(10);
    }
    saves.begin("settings", false);
   //settings values init
   saves_screen_off_time = saves.getUInt("screen_off_time", 30000); // in micro-seconds
   rtc.offset = gmt_offset = saves.getLong("gmt_offset", 3600); // in seconds
   buzzer_always_on = saves.getBool("b_always_on",true);
   buzzer_error_sound = saves.getBool("b_error_sound",false);
   buzzer_pause_sound = saves.getBool("b_pause_sound",true);
   buzzer_press_sound = saves.getBool("b_press_sound",true);
   
}




void loop() {
update_last_action();
counter_formatting();
// adding the value of each button to a seperate var
up = button_up.press();
down = button_down.press();
right = button_right.press();
left = button_left.press();
selecT = button_selecT.press(); // the name is with "T" not "t" due to interferance with another library :p
ir_button();  

if(selecT == "pressed" || down == "pressed" || up == "pressed" || right == "pressed" || left == "pressed"){
  rgb_display(6);
  buzzer_tone_select = "one_time_short";
}
if(selecT == "long_pressed" || down == "long_pressed" || up == "long_pressed" || right == "long_pressed" || left == "long_pressed"){
  rgb_display(2);
  buzzer_tone_select = "one_time_long";
  //resume_tone(buzzerpin);
}



// adding the values of bounce press vars
selecT_bounce = button_selecT.bounce_press(); 

if(selecT == "long_pressed" ){ //this is for entering the grid mode or main screen
  if(select_mode == 1){
    select_mode = 0;
  }
  else if(select_mode == 0){
    select_mode = 1;
  }
}


if(select_mode == 0 && right == "long_pressed"){
    select_mode = 6;
}
else if(select_mode == 0 && left == "long_pressed"){
  select_mode = 7;
}



if(select_mode == 6 && left == "long_pressed"){
    select_mode = 0;
    left = "";
}

if(select_mode == 7 && right == "long_pressed"){
  select_mode = 0;
}

if(select_mode == 7){
  last_action_screen();
}

if(select_mode == 6){
  temp_screen();
}



if (select_mode == 1){
  select_mode = 0;
  grid_navigiation(); // the grid navigiation & selection & invertion function
}
else if(select_mode == 0){
  main_screen();
  if(selecT == "pressed" && (select_mode == 0 || select_mode == 1) && chosen_value != "Nothing"){ // pressing sclect while screen is off doesnt change pause value (select_mode 3)
    if(paused == false){
      paused = true;
      buzzer_tone_select = "pause";
    }
  else if(paused == true){
    paused = false;
    buzzer_tone_select = "resume";
  }  
  }
}
else if(select_mode == 4) {
  display.clearDisplay();
  drop_screen();
} 

if(WiFi.status() != WL_CONNECTED && millis() > connection_begin + 3600000){ // if wifi isnt connected , try to connect every 1 hour
  connect_wifi();
}
 
 if(down == "long_pressed" && select_mode == 0){
  select_mode = 4;
 }
 else if(select_mode == 4 && up == "long_pressed"){
  if(options_select_mode == 0){
    select_mode = 0;
  }
  else{
    options_select_mode = 0;
  }
  
 }


screen_off();

if(WiFi.status() == WL_CONNECTED && server_started == false){
   web_server();
   server_started = true;
}

   

if(select_mode == 5){
  display.clearDisplay();
  error_message(error_message_text,last_select_mode);
} 






display.display();
}
 