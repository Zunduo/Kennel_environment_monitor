#include <TimerOne.h>
#include <HttpPacket.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <OneWire.h>
#include <Wire.h> 
#include <SPI.h> //Call SPI library so you can communicate with the nRF24L01+
#include <nRF24L01.h> //nRF2401 libarary found at https://github.com/tmrh20/RF24/
#include <RF24.h> //nRF2401 libarary found at https://github.com/tmrh20/RF24/
 
const int pinCE = 9; //This pin is used to set the nRF24 to standby (0) or active mode (1)
const int pinCSN = 10; //This pin is used to tell the nRF24 whether the SPI communication is a command or message to send out

RF24 wirelessSPI(pinCE, pinCSN); // Declare object from nRF24 library (Create your wireless SPI) 
const uint64_t rAddress[] = {0xB00B1E50D2LL, 0xB00B1E50C3LL};  //Create pipe addresses for the 2 nodes to recieve data, the "LL" is for LongLong type
const uint64_t wAddress[] = {0xB00B1E50B1LL, 0xB00B1E50A4LL};  //Create pipe addresses for the 2 nodes to transmit data, the "LL" is for LongLong type      
  

struct package
{
  float temperature;
  float humidity;
  float wind;
  int dogpre;
  int doghr;
  int dogmin;
  int dogsec;

};
typedef struct package Package;
Package data;
byte daNumber=80;





float Temp_Buffer = 0;       // Defining temperature float variables

HttpPacketHead packet;

#define DebugSerial Serial      //Defining program debugging connected to serial port 1
#define ESP8266Serail Serial3   //Define ESP8266 connected to serial port 3

#define Success 1U
#define Failure 0U

int L = 13;      //LED indicator pin 13

unsigned long  Time_Cont = 0;   //Timer counter

const unsigned int esp8266RxBufferLength = 600;
char esp8266RxBuffer[esp8266RxBufferLength];
unsigned int ii = 0;

const char ssid[] = "ZHAOZUNDUO";   //Define router username
const char password[] = "123123123";  //Define router password

char OneNetServer[] = "api.heclouds.com";                 //define the address of the onenet server
char device_id[] = "25683874";                            //product ID,get from the platform
char API_KEY[] = "b=S5ZQHNC5Ugb1uOw8VPMfu4b5Q=";          //API_KEY, get from the platform

char sensor_id1[] = "Temp1"; //Add temperature data flow
char sensor_id2[] = "Humi1";//Add humidity data flow
char sensor_id3[] = "Wind1"; //Add temperature data flow

char sensor_id4[] = "Temp2";//Add humidity data flow
char sensor_id5[] = "Humi2"; //Add temperature data flow
char sensor_id6[] = "Wind2";//Add humidity data flow

char sensor_id7[] = "Dogpresent1";//Add humidity data flow
char sensor_id8[] = "Hour1";//Add humidity data flow
char sensor_id9[] = "Minute1"; //Add temperature data flow
char sensor_id10[] = "Second1";//Add humidity data flow

char sensor_id11[] = "Dogpresent2";//Add humidity data flow
char sensor_id12[] = "Hour2";//Add humidity data flow
char sensor_id13[] = "Minute2"; //Add temperature data flow
char sensor_id14[] = "Second2";//Add humidity data flow





void setup() 
{
    Serial.begin(9600);
  wirelessSPI.begin();  //Start the nRF24 module
  wirelessSPI.openReadingPipe(1,rAddress[0]);      //open pipe o for recieving meassages with pipe address
  wirelessSPI.openReadingPipe(2,rAddress[1]);      //open pipe o for recieving meassages with pipe address
  wirelessSPI.startListening();                 // Stop listening for messages  
        
  pinMode(L,OUTPUT);
  digitalWrite(L,LOW);

  DebugSerial.begin(9600);  //Set debug baud rate
  ESP8266Serail.begin(115200); //Set debug esp8266 connection baud rate

  Timer1.initialize(1000); 
  Timer1.attachInterrupt(Timer1_handler);

  initEsp8266();  //Initialize Esp8266
  
 
  
  DebugSerial.println("setup end!");
  Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)"); 
}

void loop() 
{
  
        /****************************************RF data***************************************************/   
   byte pipeNum = 0; //variable to hold which reading pipe sent data
    byte gotByte = 0; //used to store payload from transmit module
    float temp1, temp2;
    float humid1, humid2;
    float wind1, wind2;
    int dogpre1, dogpre2;
    int doghr1, doghr2;
    int dogmin1, dogmin2;
    int dogsec1, dogsec2;
 
    while(wirelessSPI.available(&pipeNum)){ //Check if recieved data
     wirelessSPI.read( &data, sizeof(data )); //read one byte of data and store it in gotByte variable
     Serial.print("Recieved guess from transmitter: "); 
     Serial.println(pipeNum); //print which pipe or transmitter this is from

    if(pipeNum ==1)
    {
      temp1=data.temperature;
      humid1=data.humidity;
      wind1=data.wind;
      dogpre1=data.dogpre;
      doghr1=data.doghr;
      dogmin1=data.dogmin;
      dogsec1=data.dogsec;
     Serial.print("Alpha1 guess number: ");
     Serial.println(temp1); //print payload or the number the transmitter guessed
     Serial.println(humid1);
     Serial.println(wind1);
     
          postDataToOneNet(API_KEY,device_id,sensor_id1,temp1);              //Upload data to OneNet corresponding device sensors
          delay(200);
          postDataToOneNet(API_KEY,device_id,sensor_id2,humid1);
           delay(200);
         postDataToOneNet(API_KEY,device_id,sensor_id3,wind1);
          delay(200);
          postDataToOneNet(API_KEY,device_id,sensor_id7,dogpre1);                     //Upload data to OneNet corresponding device sensors
           delay(200);
          postDataToOneNet(API_KEY,device_id,sensor_id8,doghr1);
           delay(200);  
           postDataToOneNet(API_KEY,device_id,sensor_id9,dogmin1);                     //Upload data to OneNet corresponding device sensors
           delay(200);
          postDataToOneNet(API_KEY,device_id,sensor_id10,dogsec1);
           delay(200);

    }
    else if(pipeNum==2)
    {
      temp2=data.temperature;
      humid2=data.humidity;
      wind2=data.wind;
      dogpre2=data.dogpre;
      doghr2=data.doghr;
      dogmin2=data.dogmin;
      dogsec2=data.dogsec;
     Serial.println("Alpha2 guess number: ");
     Serial.print("\t\t\t");
     Serial.println(temp2); //print payload or the number the transmitter guessed
      Serial.print("\t\t\t");
     Serial.println(humid2);
      Serial.print("\t\t\t"); 
     Serial.println(wind2);
     postDataToOneNet(API_KEY,device_id,sensor_id4,temp2);              //Upload data to OneNet corresponding device sensors
      delay(200);
          postDataToOneNet(API_KEY,device_id,sensor_id5,humid2);                     //Upload data to OneNet corresponding device sensors
           delay(200);
          postDataToOneNet(API_KEY,device_id,sensor_id6,wind2);
           delay(200);
           postDataToOneNet(API_KEY,device_id,sensor_id11,dogpre2);                     //Upload data to OneNet corresponding device sensors
           delay(200);
           postDataToOneNet(API_KEY,device_id,sensor_id12,doghr2);
           delay(200);
          postDataToOneNet(API_KEY,device_id,sensor_id13,dogmin2);                     //Upload data to OneNet corresponding device sensors
           delay(200);
          postDataToOneNet(API_KEY,device_id,sensor_id14,dogsec2);
           delay(200);
    }
    }
            /****************************************RF data***************************************************/                                                                       
          
          
       
}



void postDataToOneNet(char* API_VALUE_temp,char* device_id_temp,char* sensor_id_temp,float thisData)  //assembly POST request
{
  
  StaticJsonBuffer<200> jsonBuffer;

  JsonObject& value = jsonBuffer.createObject();
  value["value"] = thisData;

  JsonObject& id_datapoints = jsonBuffer.createObject();
  id_datapoints["id"] = sensor_id_temp;
  JsonArray& datapoints = id_datapoints.createNestedArray("datapoints");
  datapoints.add(value);

  JsonObject& myJson = jsonBuffer.createObject();
  JsonArray& datastreams = myJson.createNestedArray("datastreams");
  datastreams.add(id_datapoints);

  char p[200];
  int num = myJson.printTo(p, sizeof(p));


  packet.setHostAddress(OneNetServer);
  packet.setDevId(device_id_temp);   //device_id
  packet.setAccessKey(API_VALUE_temp);  //API_KEY
  // packet.setDataStreamId("<datastream_id>");    //datastream_id
  // packet.setTriggerId("<trigger_id>");
  // packet.setBinIdx("<bin_index>");

  packet.createCmdPacket(POST, TYPE_DATAPOINT, p);  //create the http message about add datapoint 
  int httpLength = strlen(packet.content) + num;  

  //connnet server
  char cmd[400];
  memset(cmd, 0, 400);  //clear cmd
  strcpy(cmd, "AT+CIPSTART=\"TCP\",\"");
  strcat(cmd, OneNetServer);
  strcat(cmd, "\",80\r\n");
  if (sendCommand(cmd, "CONNECT", 10000, 5) == Success);
  else ESP8266_ERROR(1);
  clrEsp8266RxBuffer();


  //post data 
  memset(cmd, 0, 400);  //clear cmd
  sprintf(cmd, "AT+CIPSEND=%d\r\n", httpLength);
  if (sendCommand(cmd, ">", 3000, 1) == Success);
  else ESP8266_ERROR(2);
  clrEsp8266RxBuffer();

  memset(cmd, 0, 400);  //clear cmd
  strcpy(cmd, packet.content);
  strcat(cmd, p);
  if (sendCommand(cmd, "\"succ\"}", 3000, 1) == Success);
  else ESP8266_ERROR(3);
  clrEsp8266RxBuffer();

  if (sendCommand("AT+CIPCLOSE\r\n", "CLOSED", 3000, 1) == Success);
  else ESP8266_ERROR(4);
  clrEsp8266RxBuffer();
}

void initEsp8266()
{
  if (sendCommand("AT\r\n", "OK", 1000, 10) == Success);
  else ESP8266_ERROR(5);
  clrEsp8266RxBuffer();

  if (sendCommand("AT+CWMODE=1\r\n", "OK", 3000, 10) == Success);
  else ESP8266_ERROR(7);
  clrEsp8266RxBuffer();

  char cmd[50];
  strcpy(cmd, "AT+CWJAP=\"");
  strcat(cmd, ssid);
  strcat(cmd, "\",\"");
  strcat(cmd, password);
  strcat(cmd, "\"\r\n");

  if (sendCommand(cmd, "OK", 20000, 10) == Success);
  else ESP8266_ERROR(8);
  clrEsp8266RxBuffer();

}

void(* resetFunc) (void) = 0; //Manufacturing restart command

void ESP8266_ERROR(int num)
{
  DebugSerial.print("ERROR");
  DebugSerial.println(num);
  while (1)
  {
    digitalWrite(L, HIGH);
    delay(300);
    digitalWrite(L, LOW);
    delay(300);

    if (sendCommand("AT\r\n", "OK", 100, 10) == Success)
    {
      DebugSerial.print("\r\nRESET!!!!!!\r\n");
      resetFunc();
    }
  }
}



unsigned int sendCommand(char *Command, char *Response, unsigned long Timeout, unsigned char Retry)
{
  clrEsp8266RxBuffer();
  for (unsigned char n = 0; n < Retry; n++)
  {
    DebugSerial.print("\r\nsend AT Command:\r\n----------\r\n");
    DebugSerial.write(Command);

    ESP8266Serail.write(Command);

    Time_Cont = 0;
    while (Time_Cont < Timeout)
    {
      esp8266ReadBuffer();
      if (strstr(esp8266RxBuffer, Response) != NULL)
      {
        DebugSerial.print("\r\nreceive AT Command:\r\n==========\r\n");
        DebugSerial.print(esp8266RxBuffer);                         //Output received information     
        return Success;
      }
    }
    Time_Cont = 0;
  }
  DebugSerial.print("\r\nreceive AT Command:\r\n==========\r\n");
  DebugSerial.print(esp8266RxBuffer);                              //Output received information
  return Failure;
}



void Timer1_handler(void)
{
  Time_Cont++;
}



void esp8266ReadBuffer() 
{
  while (ESP8266Serail.available())
  {
    esp8266RxBuffer[ii++] = ESP8266Serail.read();
    if (ii == esp8266RxBufferLength)clrEsp8266RxBuffer();
  }
}

void clrEsp8266RxBuffer(void)
{
  memset(esp8266RxBuffer, 0, esp8266RxBufferLength);    //Clear counter reset
  ii = 0;
}
