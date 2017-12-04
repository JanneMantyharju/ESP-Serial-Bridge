

// ESP32 WiFi <-> 3x UART Bridge
// by AlphaLima
// www.LK8000.com

// Disclaimer: Don't use  for life support systems
// or any other situations where system failure may affect
// user or environmental safety.

#include <WiFi.h>
#include <config.h>

// config: ////////////////////////////////////////////////////////////

#define NUM_COM   3                 // total number of COM Ports
#define DEBUG_COM 0                 // debug output to COM0

#define UART_BAUD0 19200            // Configure UART0
#define SERIAL_PARAM0 SERIAL_8N1
#define SERIAL0_RXPIN 3 
#define SERIAL0_TXPIN 1 

#define UART_BAUD1 19200            // Configure UART1
#define SERIAL_PARAM1 SERIAL_8N1
#define SERIAL1_RXPIN 2 
#define SERIAL1_TXPIN 4 

#define UART_BAUD2 19200            // Configure UART2
#define SERIAL_PARAM2 SERIAL_8N1
#define SERIAL2_RXPIN 16 
#define SERIAL2_TXPIN 17

#define bufferSize 8192

#define MODE_AP // phone connects directly to ESP
//#define MODE_STA // ESP connects to WiFi router

#define PROTOCOL_TCP
//#define PROTOCOL_UDP
bool debug = true;

#ifdef MODE_AP
// For AP mode:
const char *ssid = "LK8000";  // You will connect your phone to this Access Point
const char *pw = "Flightcomputer"; // and this is the password
IPAddress ip(192, 168, 4, 1); // From RoboRemo app, connect to this IP
IPAddress netmask(255, 255, 255, 0);
const int port = 8880; // and this port
// You must connect the phone to this AP, then:
// menu -> connect -> Internet(TCP) -> 192.168.4.1:8880  for UART0
//                                  -> 192.168.4.1:8881  for UART1
//                                  -> 192.168.4.1:8882  for UART2
#endif

#ifdef MODE_STA
// For STATION mode:
const char *ssid = "YourSSID";  // Your ROUTER SSID
const char *pw = "password"; // and WiFi PASSWORD
const int port = 8880;
// You must connect the phone to the same router,
// Then somehow find the IP that the ESP got from router, then:
// menu -> connect -> Internet(TCP) -> [ESP_IP]:8880  for UART0
//                                  -> [ESP_IP]:8881  for UART1
//                                  -> [ESP_IP]:8882  for UART2
#endif

//////////////////////////////////////////////////////////////////////////


HardwareSerial Serial1(1);
HardwareSerial Serial2(2);
HardwareSerial* COM[NUM_COM] = {&Serial, &Serial1 , &Serial2};

#define MAX_NMEA_CLIENTS 4
#ifdef PROTOCOL_TCP
#include <WiFiClient.h>
WiFiServer server1(port);
WiFiServer server2(port+1);
WiFiServer server3(port+2);
WiFiClient client[NUM_COM][MAX_NMEA_CLIENTS];
#endif

#ifdef PROTOCOL_UDP
#include <WiFiUdp.h>
WiFiUDP udp;
IPAddress remoteIp;
#endif


uint8_t buf1[NUM_COM][bufferSize];
uint8_t i1[NUM_COM]={0,0,0};

char buf2[NUM_COM][bufferSize];
uint8_t i2[NUM_COM]={0,0,0};




void setup() {

  delay(500);
  
  COM[0]->begin(UART_BAUD0, SERIAL_PARAM0, SERIAL0_RXPIN, SERIAL0_TXPIN);
  COM[1]->begin(UART_BAUD1, SERIAL_PARAM1, SERIAL1_RXPIN, SERIAL1_TXPIN);
  COM[2]->begin(UART_BAUD2, SERIAL_PARAM2, SERIAL2_RXPIN, SERIAL2_TXPIN);
  
  if(debug) COM[DEBUG_COM]->println("\n\nLK8000 WiFi serial bridge V1.00");
  #ifdef MODE_AP 
   if(debug) COM[DEBUG_COM]->println("Open ESP Access Point mode");
  //AP mode (phone connects directly to ESP) (no router)
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ip, ip, netmask); // configure ip address for softAP 
  WiFi.softAP(ssid, pw); // configure ssid and password for softAP
  #endif


  #ifdef MODE_STA
   if(debug) COM[DEBUG_COM]->println("Open ESP Station mode");
  // STATION mode (ESP connects to router and gets an IP)
  // Assuming phone is also connected to that router
  // from RoboRemo you must connect to the IP of the ESP
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pw);
  if(debug) COM[DEBUG_COM]->print("try to Connect to Wireless network: ");
  if(debug) COM[DEBUG_COM]->println(ssid);
  while (WiFi.status() != WL_CONNECTED) {   
    delay(500);
    if(debug) COM[DEBUG_COM]->print(".");
  }
  if(debug) COM[DEBUG_COM]->println("\nWiFi connected");
  
  #endif

  #ifdef PROTOCOL_TCP
  COM[0]->println("Starting TCP Server 1");  
  if(debug) COM[DEBUG_COM]->println("Starting TCP Server 1");  
  server1.begin(); // start TCP server 
  server1.setNoDelay(true);
  COM[1]->println("Starting TCP Server 2");
  if(debug) COM[DEBUG_COM]->println("Starting TCP Server 2");  
  server2.begin(); // start TCP server 
  server2.setNoDelay(true);
  COM[2]->println("Starting TCP Server 3");
  if(debug) COM[DEBUG_COM]->println("Starting TCP Server 3");  
  server3.begin(); // start TCP server   
  server3.setNoDelay(true);
  #endif

  #ifdef PROTOCOL_UDP

  if(debug) COM[DEBUG_COM]->println("Starting UDP Server 1");
  udp.begin(port); // start UDP server 

  if(debug) COM[DEBUG_COM]->println("Starting UDP Server 2");
  udp.begin(port+1); // start UDP server 

  if(debug) COM[DEBUG_COM]->println("Starting UDP Server 3");
  udp.begin(port+); // start UDP server 
  
     
  #endif
}


void loop() {

 #ifdef PROTOCOL_TCP
  if (server1.hasClient())
  {
    for(byte i = 0; i < MAX_NMEA_CLIENTS; i++){
      //find free/disconnected spot
      if (!client[0][i] || !client[0][i].connected()){
        if(client[0][i]) client[0][i].stop();
        client[0][i] = server1.available();
        if(debug) COM[DEBUG_COM]->print("New client for COM0: "); 
        if(debug) COM[DEBUG_COM]->println(i);
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient TmpserverClient = server1.available();
    TmpserverClient.stop();
  }


  if (server2.hasClient())
  {
    for(byte i = 0; i < MAX_NMEA_CLIENTS; i++){
      //find free/disconnected spot
      if (!client[1][i] || !client[1][i].connected()){
        if(client[1][i]) client[1][i].stop();
        client[1][i] = server2.available();
        if(debug) COM[DEBUG_COM]->print("New client for COM1: ");
        if(debug) COM[DEBUG_COM]->println(i);
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient TmpserverClient = server2.available();
    TmpserverClient.stop();
  }  


  if (server3.hasClient())
  {
    for(byte i = 0; i < MAX_NMEA_CLIENTS; i++){
      //find free/disconnected spot
      if (!client[2][i] || !client[2][i].connected()){
        if(client[2][i]) client[2][i].stop();
        client[2][i] = server3.available();
        if(debug) COM[DEBUG_COM]->print("New client for COM3: ");
        if(debug) COM[DEBUG_COM]->println(i);
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient TmpserverClient = server2.available();
    TmpserverClient.stop();
  }  
#endif
 
  for(int num= 0; num < NUM_COM ; num++)
  {
    for(byte i = 0; i < MAX_NMEA_CLIENTS; i++)
    {         
      if(client[num][i]) 
      {
        while(client[num][i].available())
        {
          buf1[num][i1[num]] = (uint8_t)client[num][i].read(); // read char from client (RoboRemo app)
          if(i1[num]<bufferSize-1) i1[num]++;
        } 
        /*     
        buf1[num][i1[num]+1] = 0;              
        if(debug) COM[DEBUG_COM]->print("> Port");
        if(debug) COM[DEBUG_COM]->print(port + num);
        if(debug) COM[DEBUG_COM]->print(":");
        if(debug) COM[DEBUG_COM]->println((char*)buf1[num]);
        */
        COM[num]->write(buf1[num], i1[num]); // now send to UART(num):
        i1[num] = 0;
      }
    }
  
    if(COM[num]->available())
    {
      while(COM[num]->available())
      {     
        buf2[num][i2[num]] = (char)COM[num]->read(); // read char from UART(num)
        if(i2[num]<bufferSize-1) i2[num]++;
      }
      // now send to WiFi:
      /*
      buf2[num][i2[num]+1] = 0;    
      if(debug) COM[DEBUG_COM]->print("> UART");
      if(debug) COM[DEBUG_COM]->print(num+1);
      if(debug) COM[DEBUG_COM]->print(":");
      if(debug) COM[DEBUG_COM]->println(buf2[num]);
      */
      for(byte i = 0; i < MAX_NMEA_CLIENTS; i++)
      {   
        if(client[num][i])                     
          client[num][i].write((char*)buf2[num], i2[num]);
      }
      i2[num] = 0;
    }    
  }
}
