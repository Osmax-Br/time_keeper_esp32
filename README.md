# time_keeper_esp32
As a medical student I find it hard to track how many hours I've studied through the whole semester,I wanted a device where I can specify what I am currently studying and then store the elapsed time permanently , I wanted it as a device not an app on Android since the phone is a distraction, I didn't find any device on the internet like this so I made my own , with all of the UI and functions being written originally by me

project functionality:
- it uses multitasking , screen task,input task,output task etc... are all separate and work at the same time using free rtos 
- I made a code for a grid (I use 0.96" screen , didn't find a grid code for it so I made my own) to choose the subject you want to study
- you can edit the grid (Add\remove) from the internal server hosted on the esp32
- each activity has a separate data set , your timers wouldn't collide with each other
- the main timer task uses the internal RTC in the esp32 core , so no time drifting or CPU cycles used
- the time and date are updated from ntp
- there is a history page where you can find all the activities you've done (pause\resume) and the time of each event
- there is a drop down menu with a scroll bar to enter the settings part of the system (also the menu part wasn't available online , had to do it my self)
- you can change the gmt-offset and the screen turn off time and store them into eeprom (also the UI is original here)
- you can edit the elapsed time for each activity or reset the whole activity 
- there is a buzzer in this project , and you can change your preferences for it in the settings (always off , only when paused , only when clicked etc...)
- since the input task is one , you can use ir remote,the server,keypad,buttons at the same time , all are connected into one task
- the esp32 hosts an internal server where you could edit the grid , view the current timer in real time and wether it is paused or resumed , you can also pause or resumed the activity from within the server
- in the end of the day , you can upload the timers into a server (I host a server on my laptop , .PHP file is the server in the repo here)
- there is an error system , if you do something wrong the system will bring you an error (eg. trying to upload timers without internet connection)
- the time\date are updated each 12 hours if there is internet connection 
- no delay was used in this project 
- there is a bme280 sensor where it updates in the background and updates the esp32 server, there is also a separate screen for temp
- all of the system can be controlled via buttons(up,right,left,down,ok,long press) , you could also use a keypad or ir remote and do press and long press
- the project is running for 1 year without any failure or sudden reboot , the stability is excellent 

bear in mind that this is my first project using free rtos approach, I know this project is far far from perfect , but I would be glad if someone points out any mistakes, THANKS A LOT !

