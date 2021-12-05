/*
An IoT Etch-A-Sketch
uses 2 rotory encoders to generate x,y data points
in a svg poly line.

*/
#include <SPI.h>
#include <NativeEthernet.h>
#include <SD.h>
File dataFile;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
//10.71.148.177 home 
//IPAddress ip(10, 71, 148, 177);
IPAddress ip(10, 1, 0, 177);
EthernetServer server(80);
//Function defs
void rotors();
void rotorA(EthernetClient client);
void rotorB(EthernetClient client);

void listenClient(EthernetClient client);
void pageWrite(EthernetClient client);
void polyLineBegin(EthernetClient client);
void startPage(EthernetClient client);
void endPage(EthernetClient client);

int potX = A0;    
int potY = A1;    
 
int sensorValX = 0;  
int sensorValY = 0;  
 
int oldX = 0;
int oldY = 0;

/**Rotor Vars**/
// Rotary Encoder Input defintions
#define CLKA 2
#define DTA 3
#define SWA 4

#define CLKB 5
#define DTB 6
#define SWB 7

int Acounter = 0;
int Bcounter = 0;

int AcurrentStateCLK;
int BcurrentStateCLK;

int AlastStateCLK;
int BlastStateCLK;

String AcurrentDir ="";  //just easier than cleverly combining this string for seprate IO
String BcurrentDir ="";
   
void setup() {

  delay(10000);  //you want this delay. reason: tl;dr
  /** Set encoder pins as inputs **/
  pinMode(CLKA,INPUT);
  pinMode(DTA,INPUT);
  pinMode(SWA, INPUT_PULLUP);
  pinMode(CLKB,INPUT);
  pinMode(DTB,INPUT);
  pinMode(SWB, INPUT_PULLUP);

  // Read the initial state of CLK
  AlastStateCLK = digitalRead(CLKA);
  BlastStateCLK = digitalRead(CLKB);
  /***End Rotary Encoder vars****/
  
  Serial.begin(9600); 

  Ethernet.begin(mac, ip);

if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  server.begin();
  delay(100);
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  Serial.print("Initializing SD card...");

  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("SD Card init failed!");
    while (1); 
    }
/****SD CARD and Ethernet intialized*****/  
//if data file (on SD Card) Doesn't exist, create it. 
//if data file (on SD Card) does exist errase it and create a new one
  if (SD.exists("datalog.txt")) {
     Serial.println("datalog.txt exists: \n Removing it.");
     while(SD.remove("datalog.txt")!=1){
       Serial.println("Deleeting old file data");
       if (SD.remove("datalog.txt")==1){
          Serial.println ("old dataFile removed");
       }
     }   
  } else {
    Serial.println("datalog.txt doesn't exist.");
         }

  // open a new file and immediately close it:
  Serial.println("Creating datalog.txt...");
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  dataFile.close();

  // Check to see if the file exists:
  if (SD.exists("datalog.txt")) {
    Serial.println("datalog.txt exists.");
  } else {
    Serial.println("datalog.txt wasn't created.");
         }
} //end setup

void loop() {
 // listen for incoming clients
  EthernetClient client = server.available();
rotors();
listenClient(client);
}  //end loop  
void listenClient(EthernetClient client){
  
   if (client) {
    boolean currentLineIsBlank = true;
    while (client.connected()) {
       if (client.available()) {
          char c = client.read();
       // Serial.write(c);   //tells about the client connection
        if (c == '\n' && currentLineIsBlank) {
          pageWrite(client);
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }//end avail 
    } //end conect
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  } //end client
  }//end listener()

void pageWrite(EthernetClient client){
          startPage(client);   
          polyLineBegin(client);
          endPage(client);  
  }
void startPage(EthernetClient client){
     // doctype svg no need for html
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 1");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE svg>");
          client.println("<svg   xmlns='http://www.w3.org/2000/svg'  viewBox='0 0 227.35277 156.29323' height='6.1532764in' width='8.9508963in'><path  style='fill:#ff0000; stroke:#ff0000; stroke-width:1.32300019;stroke-miterlimit:4; stroke-dasharray:none; stroke-opacity:1' d='m 12.000819,0.66145838 c -6.2819637,0 -11.33936062,5.05739692 -11.33936062,11.33936062 V 144.29249 c 0,6.28197 5.05739692,11.33936 11.33936062,11.33936 H 215.35224 c 6.28197,0 11.33885,-5.05739 11.33885,-11.33936 V 12.000819 c 0,-6.2819637 -5.05688,-11.33936062 -11.33885,-11.33936062 z M 18.932696,12.696383 H 205.5854 c 5.02557,0 9.07128,4.04571 9.07128,9.071282 v 77.03923 c 0,5.025575 -4.04571,9.071285 -9.07128,9.071285 H 18.932696 c -5.025572,0 -9.0712819,-4.04571 -9.0712819,-9.071285 v -77.03923 c 0,-5.025572 4.0457099,-9.071282 9.0712819,-9.071282 z' /><ellipse style='fill:#ffffff;fill-opacity:1; stroke:#ff0000; stroke-width:1.32300007; stroke-miterlimit:4; stroke-dasharray:none; stroke-opacity:1' cx='21.16666'  cy='130.59085'  rx='10.583334' ry='9.0714283' /><ellipse  style='fill:#ffffff; fill-opacity:1; stroke:#ff0000; stroke-width:1.32300007; stroke-miterlimit:4; stroke-dasharray:none;stroke-opacity:1' cx='200.62982'   cy='131.57361' rx='10.583334' ry='9.0714283'/>");
          client.println("\n<polyline points='15,15");          
   }
 void endPage(EthernetClient client){
  client.print("' \nfill='none' stroke='#ff0000'/></svg>");
   }
void polyLineBegin(EthernetClient client){
//read data points from SD card
    File dataFile = SD.open("datalog.txt");
  // if the file is available, read it:
  if (dataFile) {
    while (dataFile.available()) {
      //Serial.write(dataFile.read());
      client.write(dataFile.read());
    }
    dataFile.close();
  }
}

  
int rotorA(){ //X coords
  AcurrentStateCLK = digitalRead(CLKA);
  if (AcurrentStateCLK != AlastStateCLK  && AcurrentStateCLK == 1){
     if (digitalRead(DTA) != AcurrentStateCLK) {
      Acounter --;
      }else{
       Acounter ++;
       }
     }
// Remember last CLK state
  AlastStateCLK = AcurrentStateCLK;
//X limits
if(Acounter<=15){
  Acounter=15;
  }
if(Acounter>=215){
  Acounter=215;
  }  
return Acounter;
  }//endA rotor
  
int rotorB(){ //Y coords
  BcurrentStateCLK = digitalRead(CLKB);
  if (BcurrentStateCLK != BlastStateCLK  && BcurrentStateCLK == 1){
    if (digitalRead(DTB) != BcurrentStateCLK) {
       Bcounter --;
       }else{
        Bcounter ++;
     }
    //Serial.println(Bcounter);
  }  
  BlastStateCLK = BcurrentStateCLK;
//Y limits
if(Bcounter<=15){
  Bcounter=15;
  }
if(Bcounter>=210){
  Bcounter=210;
  }  
  return Bcounter;
  }//EndB rotor

void rotors(){
//read rotors and write x,y points to datalog.txt  
 sensorValX=rotorA();
 sensorValY=rotorB();
String dataString = "";

if(sensorValX!=oldX || sensorValY!=oldY){
   dataString += ",";
   dataString += String(sensorValX);
   dataString += ", ";
   dataString += String(sensorValY);
   
   Serial.println(dataString);
   
   dataFile = SD.open("datalog.txt", FILE_WRITE);
    // if the file is available, write to it:
   if (dataFile) {
      dataFile.print(dataString);
      dataFile.close();
      //clear dataString
      dataString="";
      }else{
    // if the file isn't open, pop up an error:
    Serial.println("error opening datalog.txt");
  }
  dataFile.close();
  }else{
  ;
  }

 oldX=sensorValX;
 oldY=sensorValY;
  
}//end rotors 
