 ESP32 Night-Owl Smart Switch

Automatically turns on your smart lights within your home at night by detecting movement. It will turn them off when you leave. The code uses ESP32, Sinric Pro and PIR Motion Sensor.

 Features

Automatically detects what time it is based on IST (Indian Standard Time)
Only turns on between 6PM and 5AM. If there is any movement during the day it will be ignored.
The lights will turn on as shown below:
Light 1 turns on straight away.
Light 2 will turn on 15 seconds after creating a smooth cinema effect.
Both switches will automatically turn off after 5 minutes as everyone leaves.
It can be manually controlled via the Sinric Pro app or Alexa/Google Assistan
Components

ESP32 Dev Module
4 Channel 5V Relay Module
HC-SR501 PIR Motion Sensor

ESP32 Pin Component
GPIO 23 Relay 1
GPIO 22 Relay 2
GPIO 21 Relay 3
GPIO 19 Relay 4
GPIO 34 Motion Sensor


 Upload Settings
Board ESP32 Dev Module
Partition Scheme Huge APP (3MB No OTA/1MB SPIFFS)
Flash Mode DIO
Baud Rate 115200
