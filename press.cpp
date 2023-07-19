#include "Arduino.h"
#include "press.h"
button_press::button_press(int pin){
    pinMode(pin,INPUT_PULLUP);
    _pin = pin;
}   

String button_press::press(){
String return_var;   
_nowpress = digitalRead(_pin);
  //Serial.println(nowpress);
  if(_nowpress == 1 && _lastpress == 0){
   _lastpress = 1 ; 
   _presstime = millis();
  }
  if(_lastpress == 1 && _nowpress ==0){
    _lastpress = 0;
    if(millis()- _presstime < 300){
      if(millis() > last_press_time + 10){
    return_var = " Pressed " + String(_pin) + " ";
    last_press_time = millis();
    }
    }
    else{
         if(millis() > last_press_time + 10){
     return_var = " Long pressed "+ String(_pin) + " ";
    last_press_time = millis();
    }
     
    }
  }
 return return_var ;
}