#include "Arduino.h"
#include "press.h"
button_press::button_press(int pin){
    pinMode(pin,INPUT_PULLUP); //code to run in each object init.
    int before_last_press_time ;
    _lastpress = digitalRead(_pin);  // Initialize with current state
    _pin = pin;
   
}   




String button_press::press(){
String return_var ;   // stores return info
_nowpress = digitalRead(_pin);
  
  if(_nowpress == 1 && _lastpress == 0){ //senses the change of signal (0-->1 and 1-->0 regesters a press)
   _lastpress = 1 ; 
   _presstime = millis();
  }
  if(_lastpress == 1 && _nowpress ==0){
    _lastpress = 0;
    if(millis()- _presstime < 300){ // the duration of the normal press (changing this changes also the duration of long press)
      if(millis() > last_press_time + 50){ //for ignoring dual presses at the same time
    return_var = "pressed";
    last_press_time = millis(); //updating last time pressed
    }
    }
    else{
         if(millis() > last_press_time + 10){
    return_var = "long_pressed";
    last_press_time = millis();
    }
     
    }
  }
  
 return return_var ;
}
















