# Zumo Sumo Fight project
## Project Name
Zumo Sumo Fight
## Project Description
Zumo fight is a multiplayer game that allows two players to battle with each other inside a ring. The black electric tape represents the ring of the match, and the rest of the ring is white. During the match, the player can choose to play against Autonomous Zumo, or it can play against another human.
## Project Aims
- Be able to control zumo movement using the movement of the android phone.
- Have Zumo able to run autonomously inside the ring and charge towards other zumo when detected by the ultrasonic sensor.
- Update Zumo health data on each connected device using MQTT Protocol.
- Display live score data on the external platform.
- Once health runs out on a Zumo, a winner is declared on the external platform, and loser robot stops moving. 
## List of technologies used
- Arduino Uno
- State Machines Pattern
- Bluetooth HC-05 Module
- Raspberry Pi with a touch display.
- Node MCU 1.0 Module
- Android Device
- Android Application
- MQTT Protocol
- JavaScript, HTML and CSS
- Polulo Zumo Shield
- Ultra-sonic sensor


## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

## Software installation and setup for development
1. Install Arduino software from here:

[Arduino IDE-For Windows XP and up](https://www.arduino.cc/download_handler.php?f=/arduino-1.8.8-windows.zip)

[Arduino IDE-Mac OS X  10.8 Mountain Lion or newer](https://www.arduino.cc/download_handler.php?f=/arduino-1.8.8-macosx.zip)

[Linux  64 bits](https://www.arduino.cc/download_handler.php?f=/arduino-1.8.8-linux64.tar.xz)

2. Install Android Studio from here:

[Android Studio Windows XP and up 64 bit](https://dl.google.com/dl/android/studio/ide-zips/3.3.2.0/android-studio-ide-182.5314842-windows.zip)

[Arduino IDE-Mac OS X  10.8 Mountain Lion or newer](https://dl.google.com/dl/android/studio/install/3.3.2.0/android-studio-ide-182.5314842-mac.dmg)

[Linux  64 bits](https://dl.google.com/dl/android/studio/ide-zips/3.3.2.0/android-studio-ide-182.5314842-linux.zip)


3. If you want to edit the scoreboard web page download the software from here:
[Visual Studio Code](https://code.visualstudio.com/docs/?dv=win)

4. An Android phone is required to play this game. 

5. Node MCU drivers require installation from the following link: 
[Download MCU drivers for Windows](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)

6. Install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

7. Bluetooth Module HC-05 requires no driver installation. Although this bluetooth module is limited to Android only. It will not be detected by iOS. 

### For Zumo
#### Install Arduino Libraries
Open Arduino-IDE and in tools navigate to manage libraries.
 - search for zumo shield and install library. 
 - search for newping and install library.
 - search for ESP8266 and install library.
#### Upload ZumoControlsArduino.ino Sketch to Arduino board
 1. Open ZumoControlsArduino.ino project in Arduino IDE
 2. Connect the Arduino Uno board to the computer
 3. Select the correct board in tools -> Board
 4. Select the Serial Port in tools -> Port
 5. Upload Sketch

#### Upload NodeMCU_Arduino.ino sketch to Node MCU module
1. Open ZumoControlsArduino/NodeMCU_Arduino/NodeMCU_Arduino.ino project in Arduino IDE
2. Hold the flash button on the node module.
3. Connect the NodeMCU Module to the computer keep holding the flash button.
4. Once connected release the flash button
5. Select the NodeMCU 1.0 (ESP-12E Module) in tools -> board
6. Select the serial port in tools -> port
7. Upload the Sketch

#### For Android Studio
Open the project and build the app and then push it to your android phone. 

## Usage

Follow the given steps in order to use the Zumo:

1. Connect the MQTT broker and subscribe the topic that is in the Node MCU WiFI Module.
2. Open the Score.html and it will connect to the MQTT broker.
3. Turn on the Zumo robots and if the Node MCU has been set up correctly. It will display on the broker that Zumo has been connected. 
4. On the Android device open the android application to Enter player name and select bluetooth device showing as "HC-05". 
5. Final step is to start the Zumo by selecting "Manual Mode" or Auto Mode"
6. Pressing the Manual Mode button whilst in Auto Mode stops the Zumo. 

## Contributers 

* **Deepak Jindal**
* **Saqib Simaei**
* **Sahand Simaei**
* **Uraish Ahmed**
