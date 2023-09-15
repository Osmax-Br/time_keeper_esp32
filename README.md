# time_keeper_esp32
this project is a desk smart clock , its main purpose is to organize my time and calculate the time spent on each activity based on the data I gave , I originally started the project in 2021 and the first major release was in 2022 however in was buggy so I rewrote the code to work with esp32 using dual cores and multitasking , I used new ways to write the code and also benifited from all esp32 internal features such as RTC and timer , This project is the biggest project I wrote and it was a challenge to push my self to the boundries , the project has comments to make it easier to understand <br>
This project took me a lot of time and work , I hope you like this IOT project.
<br>
# features :
<li>main screen which has a clock that updates time for interet and you can change time zone from settings , it also displays current activity and time spent on the current activity</li>
<li>activity selection grid which you can navigate using the ir remote or buttons , you can switch between pages also</li>
<li>drop down menu which contains the settings</li>
<li>each activity has a specific place in memory to save the time spent in that activity</li>
<li>when you want to end the day the device will make you choose the upload date and the upload it to the internal server on the pc</li>
<li>there is a RGB led which changes color depending on the current situation (like notification led)</li>
<li>internal memory to save settings such as screen-off time and time zone and buzzer prefrences</li>
<li>you can reset certain activities</li>
<li>you can navigate either using the remote control or using the buttons</li>
<li>the OLED screen has a timer to turn off</li>
<li>used internal RTC to have accurate activity timer</li>
<li>it has internal temp sensor</li>
<li>it has DHT11 sensor which senses TEMP and humidity (+- 2 C  ,  +- 5%)</li><br>
#hardware :
Esp32 (4mb ROM , 256 kb Ram , wifi , bluetooth) , OLED sceen (0.96" 128*64) , ir reciever diode , RGB led , navigation buttons , DHT11 sensor
<br>
#Osama Breman 2023 

