#ifndef pressClass_h
#define pressClass_h
#include "Arduino.h"
// class config
class button_press{
public:
    button_press(int pin);
    String press();
    int bounce_press();
    int last_press_time ;
    int last_press_time_bounce = 0 ;
    int press_state = 0;
private:
    int _pin,_nowpress,_lastpress,_presstime;
    int _last_press_state = 0 ;
    int _nowpress_bounce = 0;
};



#endif