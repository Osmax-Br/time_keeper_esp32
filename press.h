#ifndef pressClass_h
#define pressClass_h
#include "Arduino.h"

class button_press{
public:
    button_press(int pin);
    String press();
    int last_press_time ;
private:
    int _pin,_nowpress,_lastpress,_presstime;
};



#endif