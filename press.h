#ifndef pressClass_h
#define pressClass_h
#include "Arduino.h"
// class config
class button_press{
public:
    button_press(int pin);
    String press();
    int bounce_press();
    int last_press_time ;   //ignoring multiple presses at the same time for press function
    int last_press_time_bounce = 0 ; //ignoring multiple presses at the same time for bounce function
private:
    int _pin,_nowpress,_lastpress,_presstime;
    int _last_press_state = 0 ; //stores bounce_press value
    int _nowpress_bounce = 0;   //stores digitalRead value 
};

// _bounce are vars for bounce function

#endif