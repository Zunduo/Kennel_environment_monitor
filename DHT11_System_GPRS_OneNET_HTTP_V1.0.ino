【接口说明】： 
               Arduino Mega 2560与GSM-SIM800C连线采用UART串口通信，与单片机串口3通信接线如下：
		      Arduino Mega 2560单片机      SIM800C 
			        RX3                  TXD
			        TX3                  RXD 
               通过Arduino Mega 2560单片机的串口0可以监视SIM800C模块的工作情况，用于将信息打印输出，串口3和GSM-SIM800C模块串口通信。
               
（1）DHT11温湿度传感器模块：D2  	
     
     	   　　 
【功能说明】：arduino和sim800C的GPRS模块进行HTTP通信，发送数据到云端
 

1.硬件实物测试使用说明：

◆第一步：整体了解熟悉整个系统的使用流程，上电进行实物的测试操作。

◆第二步：
 【1】自己准备一张SIM手机卡插入模块，注意卡的缺口方向，手机卡要求是中国移动或者联通2G、3G、4G卡，不支持中国电信卡。
 【2】利用DC 5V开关电源适配器或者5V移动电源给系统供电。上电之后SIM800C模块上面的NET红色网络指示灯以1秒的频率快速闪烁，
      此时需要等待3~5s，GSM模块自动搜索网络信号，当3S闪烁一次表明此时GSM网络通信正常，如果出现1S频繁间断闪烁，此时表明
      通信失败，此时检查插入的手机卡方向以及GSM天线和电源供电的稳定性。    
      
◆第三步：安装我们提供的OneNET安卓版本APP软件，直接安装到安卓手机，即可使用！

◆第四步：打开OneNET的手机APP软件，或者电脑PC网页端，或者微信公众平台，登陆我们提供的测试账号，用户名：****** 密码：****** ，必须填写一致，否则无法使用。 

2.软件程序修改与下载烧录使用说明：

（1）把资料中提供的所有库文件文件夹复制到arduino开发环境的libraries文件夹下。

    ①.使用DHT11温湿度传感器的库<dht11.h>，读取温湿度值。
    ②.使用定时器<TimerOne.h>库，监控AT指令发送和获取返回数据的时间。

（2）在使用OneNet云平台时，对应自己注册的账户，需要修改相关的程序，切记！不修改不行！修改部分如下：
          
          char device_id[] = "********";        //修改为自己的设备ID
          char API_KEY[] = "********";	        //修改为自己的API_KEY

（3）修改完成程序之后，插上Mega2560的电路板，选择正确的板卡型号Arduino Mega 2560，最后把代码下载进即可，重新上电观看自己的账户传感器的效果。


#include <TimerOne.h>
#include <dht11.h>
//#include <SoftwareSerial.h>

dht11 DHT11;
#define DHT11PIN 2                 //DHT11温湿度传感器接在引脚2

//#define DebugSerial  Serial
//SoftwareSerial GprsSerial(10, 11); // RX, TX	

#define DebugSerial Serial      //定义Debug程序调试接在串口0
#define GprsSerial  Serial3	//定义GPRS通信模块接在串口3

#define Success 1U
#define Failure 0U

int L = 13;	                 //LED指示灯引脚13

unsigned long  Time_Cont = 0;	 //定时器计数器`

const unsigned int gprsRxBufferLength = 600;
char gprsRxBuffer[gprsRxBufferLength];
unsigned int gprsBufferCount = 0;


char OneNetServer[] = "api.heclouds.com";		//不需要修改
char device_id[] = "25707248";	                        //修改为自己的设备ID
char API_KEY[] = "2uT=B2t=hBabmvTV0CF2y38yEeM=";	//修改为自己的API_KEY

char sensor_id1[] = "Temp";        //添加温度数据流
char sensor_id2[] = "Humi";        //添加湿度数据流

void setup() 
{
	pinMode(L,OUTPUT);
	digitalWrite(L,LOW);

	DebugSerial.begin(9600);
	GprsSerial.begin(9600);

	Timer1.initialize(1000);
	Timer1.attachInterrupt(Timer1_handler);

	initGprs();		//初始化模块

	DebugSerial.println("\r\nsetup end!");

}

void loop() 
{
	  /****************************************DHT11温湿度*****************************************************/ 
	  int chk = DHT11.read(DHT11PIN);//读取温湿度值

	  //串口调试DHT11输出信息
	  DebugSerial.print("Read sensor: ");
	  switch (chk)
	  {
  	  case DHTLIB_OK:
  	    DebugSerial.println("OK");  
  	    break;
  	  case DHTLIB_ERROR_CHECKSUM:
  	    DebugSerial.println("Checksum error");
  	    break;
  	  case DHTLIB_ERROR_TIMEOUT:
  	    DebugSerial.println("Time out error");
  	    break;
  	  default:
  	    DebugSerial.println("Unknown error");
  	    break;
	  }    
 	/*********************************数据上传到OneNET物联网云端************************************************/ 
        postDataToOneNet(API_KEY,device_id,sensor_id1,DHT11.temperature);		
        postDataToOneNet(API_KEY,device_id,sensor_id2,DHT11.humidity); 
        delay(100);
          
}

void postDataToOneNet(char* API_VALUE_temp, char* device_id_temp, char* sensor_id_temp, double data_value)
{
	char send_buf[400] = {0};
	char text[100] = {0};
	char tmp[25] = {0};

	char value_str[15] = {0};
	//Arduino平台不支持sprintf()来将浮点数转换为字符串，只能使用dtostrf()函数将浮点型数转换为字符串类型
        //dtostrf(floatVar, minStringWidthIncDecimalPoint, numVarsAfterDecimal, charBuf);  
	dtostrf(data_value, 3, 2, value_str); //将浮点型数转换为字符串类型输出

	//连接服务器
	memset(send_buf, 0, 400);    //清空
	strcpy(send_buf, "AT+CIPSTART=\"TCP\",\"");    //建立 TCP/IP 连接
	strcat(send_buf, OneNetServer);
	strcat(send_buf, "\",80\r\n");
	if (sendCommand(send_buf, "CONNECT OK\r\n", 10000, 5) == Success);
	else errorLog(9);

	//发送数据
	if (sendCommand("AT+CIPSEND\r\n", ">", 3000, 1) == Success);//模块向服务器发送数据请求
	else errorLog(10);

	memset(send_buf, 0, 400);    //清空

	/*准备JSON串*/
	//sprintf()函数用于将格式化的数据写入字符串，%s用于将整数转成字符串然后打印到字符串
	sprintf(text,"{\"datastreams\":[{\"id\":\"%s\",\"datapoints\":[{\"value\":%s}]}]}",sensor_id_temp,value_str);

	/*准备HTTP报头*/
	send_buf[0] = 0;
	strcat(send_buf, "POST /devices/");
	strcat(send_buf, device_id_temp);
	strcat(send_buf, "/datapoints HTTP/1.1\r\n"); //注意后面必须加上\r\n
	strcat(send_buf, "api-key:");
	strcat(send_buf, API_VALUE_temp);
	strcat(send_buf, "\r\n");                     //注意后面必须加上\r\n
	strcat(send_buf, "Host:");
	strcat(send_buf, OneNetServer);
	strcat(send_buf, "\r\n");                     //注意后面必须加上\r\n
	sprintf(tmp, "Content-Length:%d\r\n\r\n", strlen(text)); //sprintf()函数用于将格式化的数据写入字符串，%d 用于将整数转成十进制,计算JSON串长度
	strcat(send_buf, tmp);
	strcat(send_buf, text);

	if (sendCommand(send_buf, send_buf, 3000, 1) == Success);
	else errorLog(11);
       
	char sendCom[2] = {0x1A};    //确定发送
	if (sendCommand(sendCom, "\"succ\"}", 3000, 1) == Success);
	else errorLog(12);

	if (sendCommand("AT+CIPCLOSE=1\r\n", "CLOSE OK\r\n", 3000, 1) == Success);  //关闭当前 TCP 连接
	else errorLog(13);

	if (sendCommand("AT+CIPSHUT\r\n", "SHUT OK\r\n", 3000, 1) == Success);  //关闭移动场景
	else errorLog(14);
}


void initGprs()
{
	if (sendCommand("AT\r\n", "OK\r\n", 3000, 10) == Success);   //AT指令初始化  测试GSM是否启动
	else errorLog(1);

	if (sendCommand("AT+CPIN?\r\n", "OK\r\n", 1000, 10) == Success); //查询SIM卡是否被识别
	else errorLog(2);

	if (sendCommand("AT+CREG?\r\n", "OK\r\n", 3000, 10) == Success); //查询SIM卡是否注册网络
	else errorLog(3);

	if (sendCommand("AT+CGCLASS=\"B\"\r\n", "OK\r\n", 3000, 2) == Success);  //选择移动台类别为B
	else errorLog(4);

	if (sendCommand("AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n", "OK\r\n", 3000, 2) == Success);//定义PDP上下文表示，选择接入点：移动：CMNET 联通：UNINET
	else errorLog(5);

	if (sendCommand("AT+CGATT=1\r\n", "OK\r\n", 3000, 2) == Success);  //设置附着GPRS网络
	else errorLog(6);

	if (sendCommand("AT+CIPCSGP=1,\"CMNET\"\r\n", "OK\r\n", 3000, 2) == Success);  //设置GPRS链接模式，接入点为CMNET
	else errorLog(7);

	if (sendCommand("AT+CLPORT=\"TCP\",\"2000\"\r\n", "OK\r\n", 3000, 2) == Success);   //设置TCP连接，端口号为2000  
	else errorLog(8);
}

void(* resetFunc) (void) = 0; //制造重启命令

void errorLog(int num)
{
	DebugSerial.print("ERROR");
	DebugSerial.println(num);

	while (1)
	{
		digitalWrite(L,HIGH);
		delay(100);
		digitalWrite(L,LOW);
		delay(100);
		digitalWrite(L,HIGH);
		delay(100);
		digitalWrite(L,LOW);
		delay(300);

		if (sendCommand("AT\r\n", "OK\r\n", 100, 10) == Success)
		{
			DebugSerial.print("\r\nRESET!!!!!!\r\n");
			resetFunc();
		}
	}
}

unsigned int sendCommand(char *Command, char *Response, unsigned long Timeout, unsigned char Retry)
{
	clrGprsRxBuffer();
	for (unsigned char n = 0; n < Retry; n++)
	{
		DebugSerial.print("\r\n==========send AT Command:==========\r\n");
		DebugSerial.write(Command);

		GprsSerial.write(Command);

		Time_Cont = 0;
		while (Time_Cont < Timeout)
		{
			gprsReadBuffer();
			if (strstr(gprsRxBuffer, Response) != NULL)
			{
				DebugSerial.print("\r\n==========receive AT Command:==========\r\n");
				DebugSerial.print(gprsRxBuffer); //输出接收到的信息
				return Success;
			}
		}
		Time_Cont = 0;
	}
	DebugSerial.print("\r\n==========receive AT Command:==========\r\n");
	DebugSerial.print(gprsRxBuffer);//输出接收到的信息
	return Failure;
}

unsigned int sendCommandReceive2Keyword(char *Command, char *Response, char *Response2, unsigned long Timeout, unsigned char Retry)
{
	clrGprsRxBuffer();
	for (unsigned char n = 0; n < Retry; n++)
	{
		DebugSerial.print("\r\n---------send AT Command:---------\r\n");
		DebugSerial.write(Command);

		GprsSerial.write(Command);

		Time_Cont = 0;
		while (Time_Cont < Timeout)
		{
			gprsReadBuffer();
			if (strstr(gprsRxBuffer, Response) != NULL && strstr(gprsRxBuffer, Response2) != NULL)
			{
				DebugSerial.print("\r\n==========receive AT Command:==========\r\n");
				DebugSerial.print(gprsRxBuffer); //输出接收到的信息
				return Success;
			}
		}
		Time_Cont = 0;
	}
	DebugSerial.print("\r\n==========receive AT Command:==========\r\n");
	DebugSerial.print(gprsRxBuffer);//输出接收到的信息
	return Failure;
}


void Timer1_handler(void)
{
	Time_Cont++;
}

void gprsReadBuffer() 
{
	while (GprsSerial.available())
	{
		gprsRxBuffer[gprsBufferCount++] = GprsSerial.read();
		if (gprsBufferCount == gprsRxBufferLength) clrGprsRxBuffer();
	}
}

void clrGprsRxBuffer(void)
{
	memset(gprsRxBuffer, 0, gprsRxBufferLength);      //清空
	gprsBufferCount = 0;
}
