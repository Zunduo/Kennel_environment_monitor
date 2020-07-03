
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
  float battlife;
  float heatindex;
  int dogpres;
  int a;
  int b;
  int c;
  int sam;
};
typedef struct package Package;
Package data;
byte daNumber=80;



HttpPacketHead packet;

#define DebugSerial Serial      //定义程序调试接在串口1
#define ESP8266Serail Serial3   //定义ESP8266接在串口3

#define Success 1U
#define Failure 0U

int L = 13;	    //LED指示灯引脚13

unsigned long  Time_Cont = 0;		//定时器计数器

const unsigned int esp8266RxBufferLength = 600;
char esp8266RxBuffer[esp8266RxBufferLength];
unsigned int ii = 0;

const char ssid[] = "ZHAOZUNDUO";		//修改为自己的路由器用户名
const char password[] = "123123123";	//修改为自己的路由器密码

char OneNetServer[] = "api.heclouds.com";		//不需要修改
char device_id[] = "25683874";                            //product ID,get from the platform
char API_KEY[] = "b=S5ZQHNC5Ugb1uOw8VPMfu4b5Q=";                          //修改为自己的API_KEY

char sensor_id1[] = "Temp1"; //Add temperature data flow
char sensor_id2[] = "Humi1";//Add humidity data flow
char sensor_id3[] = "Wind1"; //Add temperature data flow
char sensor_id4[] = "Dogpresent1";//Add humidity data flow
char sensor_id5[] = "Hour1";//Add humidity data flow
char sensor_id6[] = "Minute1"; //Add temperature data flow
char sensor_id7[] = "Second1";//Add humidity data flow
char sensor_id8[] = "Battery1";//Add humidity data flow
char sensor_id9[] = "Heatindex1";//Add humidity data flow
char sensor_id10[] = "sample1";//Add humidity data flow      //添加温度数据流
char sensor_id11[] = "Temp2";//Add humidity data flow
char sensor_id12[] = "Humi2"; //Add temperature data flow
char sensor_id13[] = "Wind2";//Add humidity data flow
char sensor_id14[] = "Dogpresent2";                      //Add dogpresent data flow
char sensor_id15[] = "Hour2";                            //Add humidity data flow
char sensor_id16[] = "Minute2";                          //Add temperature data flow
char sensor_id17[] = "Second2";                          //Add humidity data flow
char sensor_id18[] = "Battery2";//Add humidity data flow
char sensor_id19[] = "Heatindex2";//Add humidity data flow
char sensor_id20[] = "sample2";//Add humidity data flow

void setup() 
{   Serial.begin(9600);
  wirelessSPI.begin();  //Start the nRF24 module
  wirelessSPI.openReadingPipe(1,rAddress[0]);      //open pipe o for recieving meassages with pipe address
  wirelessSPI.openReadingPipe(2,rAddress[1]);      //open pipe o for recieving meassages with pipe address
  wirelessSPI.startListening();                 // Stop listening for messages
      
        
	pinMode(L,OUTPUT);
	digitalWrite(L,LOW);

	DebugSerial.begin(9600);
	ESP8266Serail.begin(115200);

	Timer1.initialize(1000);
	Timer1.attachInterrupt(Timer1_handler);

	initEsp8266();

	DebugSerial.println("setup end!");
  
}

void loop() 
{
  byte pipeNum = 0; //variable to hold which reading pipe sent data
    byte gotByte = 0; //used to store payload from transmit module
    float temp1, temp2;
    float humid1, humid2;
    float wind1, wind2;
    float heat1,heat2;
    float bt1,bt2;
    int a1,a2;
    int b1,b2;
    int c1,c2;
    int dog1,dog2;
    int sam1,sam2;
 while(wirelessSPI.available(&pipeNum)){ //Check if recieved data
     wirelessSPI.read( &data, sizeof(data )); //read one byte of data and store it in gotByte variable
     Serial.print("Recieved guess from transmitter: "); 
     Serial.println(pipeNum); //print which pipe or transmitter this is from

    if(pipeNum ==1)
    {
      temp1=data.temperature;
      humid1=data.humidity;
      wind1=data.wind;
      bt1=data.battlife;
      heat1=data.heatindex;
      dog1=data.dogpres;
      a1=data.a;
      b1=data.b;
      c1=data.c;
      sam1=data.sam;
          postDataToOneNet(API_KEY,device_id,sensor_id1,temp1);                       //Upload data to OneNet corresponding device sensors
          postDataToOneNet(API_KEY,device_id,sensor_id2,humid1);
          postDataToOneNet(API_KEY,device_id,sensor_id3,wind1);
          postDataToOneNet(API_KEY,device_id,sensor_id4,dog1);                     //Upload data to OneNet corresponding device sensors
          postDataToOneNet(API_KEY,device_id,sensor_id5,a1);
          postDataToOneNet(API_KEY,device_id,sensor_id6,b1);                     //Upload data to OneNet corresponding device sensors
          postDataToOneNet(API_KEY,device_id,sensor_id7,c1);
          postDataToOneNet(API_KEY,device_id,sensor_id8,bt1);
          postDataToOneNet(API_KEY,device_id,sensor_id9,heat1);                     //Upload data to OneNet corresponding device sensors
          postDataToOneNet(API_KEY,device_id,sensor_id10,sam1);   
    }
    else if(pipeNum==2)
    {
      temp2=data.temperature;
      humid2=data.humidity;
      wind2=data.wind;
      bt2=data.battlife;
      heat2=data.heatindex;
      dog2=data.dogpres;
      a2=data.a;
      b2=data.b;
      c2=data.c;
      sam2=data.sam;
          postDataToOneNet(API_KEY,device_id,sensor_id11,temp2);                       //Upload data to OneNet corresponding device sensors
          postDataToOneNet(API_KEY,device_id,sensor_id12,humid2);
          postDataToOneNet(API_KEY,device_id,sensor_id13,wind2);
          postDataToOneNet(API_KEY,device_id,sensor_id14,dog2);                     //Upload data to OneNet corresponding device sensors
          postDataToOneNet(API_KEY,device_id,sensor_id15,a2);
          postDataToOneNet(API_KEY,device_id,sensor_id16,b2);                     //Upload data to OneNet corresponding device sensors
          postDataToOneNet(API_KEY,device_id,sensor_id17,c2);
          postDataToOneNet(API_KEY,device_id,sensor_id18,bt2);
          postDataToOneNet(API_KEY,device_id,sensor_id19,heat2);                     //Upload data to OneNet corresponding device sensors
          postDataToOneNet(API_KEY,device_id,sensor_id20,sam2);
    }
  delay(200);    
}
            
        		



}

void postDataToOneNet(char* API_VALUE_temp,char* device_id_temp,char* sensor_id_temp,float thisData)
{
	//合成POST请求
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

	/*create the http message about add datapoint */
	packet.createCmdPacket(POST, TYPE_DATAPOINT, p);
	// if (strlen(packet.content))
	// 	Serial.print(packet.content);
	// Serial.print(p);
	int httpLength = strlen(packet.content) + num;	

	//连接服务器
	char cmd[400];
	memset(cmd, 0, 400);	//清空cmd
	strcpy(cmd, "AT+CIPSTART=\"TCP\",\"");
	strcat(cmd, OneNetServer);
	strcat(cmd, "\",80\r\n");
	if (sendCommand(cmd, "CONNECT", 10000, 5) == Success);
	else ESP8266_ERROR(1);
	clrEsp8266RxBuffer();


	//发送数据
	memset(cmd, 0, 400);	//清空cmd
	sprintf(cmd, "AT+CIPSEND=%d\r\n", httpLength);
	if (sendCommand(cmd, ">", 3000, 1) == Success);
	else ESP8266_ERROR(2);
	clrEsp8266RxBuffer();

	memset(cmd, 0, 400);	//清空cmd
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

void(* resetFunc) (void) = 0; //制造重启命令

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
				DebugSerial.print(esp8266RxBuffer);	//输出接收到的信息				
				return Success;
			}
		}
		Time_Cont = 0;
	}
	DebugSerial.print("\r\nreceive AT Command:\r\n==========\r\n");
	DebugSerial.print(esp8266RxBuffer);//输出接收到的信息
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
	memset(esp8266RxBuffer, 0, esp8266RxBufferLength);		//清空
	ii = 0;
}
