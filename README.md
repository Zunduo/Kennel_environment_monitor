# Kennel_environment_monitor
State of art kennel environment monitor with online platform monitor panel

Basic working principle explanation and SIM800C module usage guide:
【Interface Description】: 
               The connection between Arduino Mega 2560 and GSM-SIM800C adopts UART serial port communication, and the communication connection with serial port 3 of microcontroller is as follows:
Arduino Mega 2560 MCU SIM800C
RX3 TXD
TX3 RXD
               Through the serial port 0 of the Arduino Mega 2560 microcontroller, you can monitor the working status of the SIM800C module, which is used to print out information, and the serial port 3 communicates with the GSM-SIM800C module serial port.
               
(1) DHT11 temperature and humidity sensor module: D2
     
     ​
[Function description]: arduino communicates with the GPRS module of sim800C through HTTP and sends data to the cloud
 

1. Instructions for use of hardware physical test:

◆The first step: Understand and be familiar with the use process of the entire system, and power on the test operation of the physical object.

◆The second step:
 [1] Prepare a SIM mobile phone card to insert into the module, pay attention to the direction of the notch of the card, the mobile phone card requires China Mobile or China Unicom 2G, 3G, 4G card, China Telecom card is not supported.
 【2】Use DC 5V switching power adapter or 5V mobile power supply to supply power to the system. After power-on, the NET red network indicator on the SIM800C module flashes rapidly at a frequency of 1 second.
      At this time, you need to wait for 3~5s, and the GSM module automatically searches for network signals. When the 3S flashes once, it indicates that the GSM network communication is normal at this time. If the 1S flashes frequently and intermittently, it indicates that
      If the communication fails, check the direction of the inserted mobile phone card and the stability of the GSM antenna and power supply.
      
◆The third step: Install the OneNET Android version APP software, install it directly on the Android phone, and you can use it!

◆Step 4: Open OneNET's mobile APP software, or computer PC web page,

2. Instructions for software program modification and downloading and burning:

(1) Copy all the library file folders provided in the data to the libraries folder of the arduino development environment.

    ①. Use the library <dht11.h> of the DHT11 temperature and humidity sensor to read the temperature and humidity values.
    ②. Use the timer <TimerOne.h> library to monitor the time of AT command sending and getting the returned data.

(2) When using the OneNet cloud platform, the relevant programs need to be modified corresponding to the account registered by oneself. The modified part is as follows:
          
          char device_id[] = "********"; //Modify to your own device ID
          char API_KEY[] = "********"; //Modify to your own API_KEY

(3) After modifying the program, plug in the Mega2560 circuit board, select the correct board model Arduino Mega 2560, and finally download the code into it, then power on again to see the effect of your account sensor.
