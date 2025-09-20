#ifndef pressClass_h
#define pressClass_h
#include "Arduino.h"
// class config
class button_press{
public:
    button_press(int pin);
    String press();
    //String press_mask();
    int last_press_time ;   //ignoring multiple presses at the same time for press function
    int before_last_press_time = 0;
private:
    int _pin,_nowpress,_lastpress,_presstime;
    int debounce_time = 0;
    int _press_time = 0;

};

// _bounce are vars for bounce function

#endif