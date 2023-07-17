#include "Arduino.h"
#include "press.h"
button_press::button_press(int pin){
    pinMode(pin,INPUT_PULLUP);
    _pin = pin;
}   

String button_press::press(int pin){
String return_var;   
_nowpress = digitalRead(pin);
  //Serial.println(nowpress);
  if(_nowpress == 1 && _lastpress == 0){
   _lastpress = 1 ; 
   _presstime = millis();
  }
  if(_lastpress == 1 && _nowpress ==0){
    _lastpress = 0;
    if(millis()- _presstime < 300){
    return_var = "Pressed!";
    }
    else{
      return_var = "Long pressed";
    }
  }
 return return_var ;
}