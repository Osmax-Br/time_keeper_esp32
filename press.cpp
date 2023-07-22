#include "Arduino.h"
#include "press.h"
button_press::button_press(int pin){
    pinMode(pin,INPUT_PULLUP); //code to run in each object init.
    _pin = pin;
   
}   

String button_press::press(){
String return_var;   // stores return info
_nowpress = digitalRead(_pin);
  
  if(_nowpress == 1 && _lastpress == 0){ //senses the change of signal (0-->1 and 1-->0 regesters a press)
   _lastpress = 1 ; 
   _presstime = millis();
  }
  if(_lastpress == 1 && _nowpress ==0){
    _lastpress = 0;
    if(millis()- _presstime < 300){
      if(millis() > last_press_time + 10){ //for ignoring dual presses at the same time
    return_var = "pressed";
    last_press_time = millis();
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



int button_press::bounce_press(){
  _nowpress_bounce = digitalRead(_pin);
  if(_nowpress_bounce == HIGH){
      if(_last_press_state == 0){
        if(millis() > last_press_time_bounce + 100){
              _last_press_state = 1;
              last_press_time_bounce = millis();
        }
      }
      else if(_last_press_state == 1){
          if(millis() > last_press_time_bounce + 100){
              _last_press_state = 0;
              last_press_time_bounce = millis();
        }


      }

  }
  
 return _last_press_state;
}