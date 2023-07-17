#include <Arduino.h>
#include <press.h>
button_press button35(35);
button_press button34(34);
void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200);
 }

void loop() {
  Serial.print(button35.press(35));
  //Serial.print();
  Serial.print(button34.press(34));

}
 