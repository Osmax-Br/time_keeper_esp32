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
//#include <Wire.h>
//#include <WiFiUdp.h>
#include "html.h" // the website code
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Time.h> // for using the internal rtc
/*

INSERT INTO activities (chosen_activity,hours_passed,minutes_passed,seconds_passed,activity_date_month,activity_date_day_number,activity_date_weekday,activity_date_hour,activity_date_minute)
      VALUES ('Paul', 32, 32, 32 , 1 ,1 , "wed" ,1 ,1 );
*/
//**********************************************

//oled screen init
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int last_time_screen_on = 0;


//************************************************
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

//************************************************************
// rtc vars

ESP32Time rtc(3600); // utc time offset , here in Syria it is 1 hour = 3600 sec
bool rtc_time_updated = false;  // for not updating the time twice from the internet
int last_rtc_update = 0;


//**************************************************************
//network credentials
const char *ssid     = "lemone"; 
const char *password = "Hta87#Mi00";
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
int last_press_time = 0; // for ignoring multiple presses at the same moment
String up,down,right,left,selecT = "" ; // strores button values
int up_bounce,down_bounce,righ_bounce,left_bounce,selecT_bounce = 0;  // stores bounce press values

//****************************************************************
// internal rtc timer vars

hw_timer_t * timer = NULL; //setting up the timer
volatile int seconds_passed; // storing the counter seconds in SRAM for faster execution
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile int minutes_passed,hours_passed = 0; //storing the counter values
char time_counter_string[10] ;  // storing the formatted counter time
volatile bool paused = true ; // for pausing timer



//**************************************************************
//storage vars
int data_storage[4][13][3] ;
int data_storage_index = 0;


//****************************************************************
// web server vars
WiFiClient  clientt;
AsyncWebServer server(80);
String timer_web_var(){
  return time_counter_string;
}

String chosen_value_web_var(){
  return chosen_value;

}
String pause_web_var(){
  if(paused == true){
    if(data_storage[current_page][data_storage_index][0] == 0 && data_storage[current_page][data_storage_index][1] && data_storage[current_page][data_storage_index][2]){
      return "Start";}
    else{  
    return "Resume";}
  }
  else if(paused == false ){
    return "Pause";
  }
 else{
  return "ERROR !";
 } 
}
String var_processor(const String& var){
  //Serial.println(var);
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


//************************************************************
// sql vars
const char* sql_server = "http://192.168.1.11/esp32sql/data_base_script.php"; //must add year to database



//********************************************************
//drop screen vars
String options_list[4][4] = {{"save today","change location","turn server off","change clock"},{"ssid","password","server","screen turn off"},{"alarm","email","",""},{"","","",""}};
int cursor_place = 1;
int options_outer_counter = 0;
int block_cursor = 0;




void drop_screen(){
  
  for(int i =0 ;i<=4 ; i++){
    if(i==block_cursor){
      display.fillRect(0,16*i,110,16,WHITE);
    }
    else{
      display.drawRect(0,16*i,110,16,WHITE);}
      }
  int options_inner_counter = 0;   
  for(int i=5;i<=50;i+=15){
     display.setCursor(9,i+1);
     if(options_inner_counter == block_cursor){
      display.setTextColor(BLACK);
     }
     else{
      display.setTextColor(WHITE);
     }
    display.print(options_list[options_outer_counter][options_inner_counter]);
    //Serial.println(options_inner_counter);
    options_inner_counter++;
    }
  
  
  for(int i=0 ; i <=64 ; i+=8){   // the dots
    display.fillRect(122,i,2,2,WHITE);
          }

  if(up == "pressed"){
    if(options_outer_counter + block_cursor != 0){
    block_cursor --;}


    if(block_cursor == -1  && cursor_place !=1){
      block_cursor = 3;
    cursor_place -= 10;
    options_outer_counter--;}
  }
  if(down == "pressed"){
    if(options_outer_counter == 3 && block_cursor ==3){}
    else{
    block_cursor ++ ;}
    if(block_cursor == 4  && cursor_place != 31 && options_outer_counter != 4){
      block_cursor = 0;
    cursor_place += 10; 
    options_outer_counter++;}
  }
  display.fillRoundRect(120,cursor_place,5,20,5,WHITE);
  
  
  display.display();









}















// must add last_time var
void post() { // for pushing the data into the sqlite data base on the pc
// on pc I used wimp to host a server
 
 // http post
    if(WiFi.status()== WL_CONNECTED){ 
      HTTPClient http;
      
   
      http.begin(sql_server);
      	
      http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header
     
     char post_message[500] ; //storing the post message
     // should change the values in the next line to varaibles
     sprintf(post_message,"chosen_activity=%s&hours_passed=%i&minutes_passed=%i&seconds_passed=%i&activity_date_month=%i&activity_date_day_number=%i&activity_date_weekday=%s&activity_date_hour=%i&activity_date_minute=%i","football",24,12,6,6,2,"wed",3,5);
      int httpResponseCode = http.POST(post_message);   
      if(httpResponseCode>0){
  
    String response = http.getString();  //Get the response to the request
  
    //Serial.println(httpResponseCode);   //Print return code
    Serial.println(response);           //Print request answer
  
    }else{
  
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  
}
      
      http.end();
    }}

void get(){
if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      String serverPath = sql_server;
      
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }



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
      }
    else if(paused == false){
        paused = true;
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
      }
    request->send(200, "text/html", " <meta http-equiv='refresh' content='0; URL=http://192.168.1.199/'> ");
  });
 
  server.begin();
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
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {  // ensuring no error happened
  }
  display.print("connecting.......");
  display.display();
  connection_begin = millis();  //for timing out
     if(WiFi.status() != WL_CONNECTED){
      while(WiFi.status() != WL_CONNECTED && millis() <= connection_begin+20000){
        WiFi.begin(ssid, password);
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
    display.clearDisplay(); //without it the lables will over lap
    display.fillRect(rect_cord[x][y][0],rect_cord[x][y][1],rect_cord[x][y][2],rect_cord[x][y][3], WHITE); //making the rectangular white



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
  display.print(time_counter_string);
  display.setTextSize(1);
  display.setCursor(0,20+15);
  if(rtc_time_updated == true){
 display.print(rtc.getTime("%I:%M:%S %p %a %d/%m")); //printing current time from the rtc
  }
  else{
    display.print("Error! cant get time"); 
  }

}


void screen_off(){

 if((right=="pressed" || left == "pressed" || up == "pressed" || down == "pressed" || selecT == "pressed") && select_mode == 3){  // wake the screen up
  //for not interfering with the grid selection
  display.ssd1306_command(SSD1306_DISPLAYON);
  select_mode = 0;    // back to main screen
  last_time_screen_on = millis();
  }  
if(last_time_screen_on+30000<millis()){  // change the auto display_off time 
  select_mode = 3; // that means the screen is off
  display.ssd1306_command(SSD1306_DISPLAYOFF); // switch the display off
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

void setup() {
  sprintf(time_counter_string,"%02i:%02i:%02i",data_storage[current_page][data_storage_index][2],data_storage[current_page][data_storage_index][1],data_storage[current_page][data_storage_index][0]);  // for initilizing the counter string

  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);



  timer = timerBegin(0, 80, true);           	// timer 0, prescalar: 80, UP counting
  timerAttachInterrupt(timer, &onTimer, true); 	// Attach interrupt
  timerAlarmWrite(timer,1000000, true);  		// Match value= 1000000 for 1 sec. delay.
  timerAlarmEnable(timer);           			// Enable Timer with interrupt (Alarm Enable)


  //connecting to wifi , the ip address is 192.168.1.199
    connect_wifi();
    display.clearDisplay();

 
   xTaskCreatePinnedToCore(get_ntp_time,"ntp_time",10000,NULL,0,&ntp_time,1); // create a seperate task for getting the ntp time
if(WiFi.status() == WL_CONNECTED){
    web_server();
   }

}




void loop() {
counter_formatting();
// adding the value of each button to a seperate var
up = button_up.press();
down = button_down.press();
right = button_right.press();
left = button_left.press();
selecT = button_selecT.press(); // the name is with "T" not "t" due to interferance with another library :p
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


if (select_mode == 1){
  select_mode = 0;
  grid_navigiation(); // the grid navigiation & selection & invertion function
}
else if(select_mode == 0){
  main_screen(chosen_value);
  if(selecT == "pressed" && (select_mode == 0 || select_mode == 1) && chosen_value != "Nothing"){ // pressing sclect while screen is off doesnt change pause value (select_mode 3)
    if(paused == false){
      paused = true;
    }
  else if(paused == true){
    paused = false;
  }  
  }
}
else if(select_mode == 4) {
  //Serial.print("drop");
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
  select_mode = 0;
 }


 
//Serial.println(select_mode);

screen_off();

display.display();
}
 