
#include <SPI.h>
#include <NativeEthernet.h>
#include <SD.h>
File dataFile;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
//10.71.148.177 home 
IPAddress ip(10, 71, 148, 177);
//IPAddress ip(10, 1, 0, 177);
EthernetServer server(80);
//Function defs
void rotorSense(EthernetClient client);
void rotorA(EthernetClient client);
void rotorB(EthernetClient client);
void startPage(EthernetClient client);
void endPage(EthernetClient client);
void svgHeader(EthernetClient client);
void svgFooter(EthernetClient client);
void polyLineBegin(EthernetClient client);
void polyLineEnd(EthernetClient client);
void pageWrite(EthernetClient client);
void listenClient(EthernetClient client);
void background(EthernetClient client); //only want to do this one once, maybe I can run an Iframe or something
void sensors();//pots
/**Pot Vars**/ 
int potX = A0;    
int potY = A1;    
 
int sensorValX = 0;  
int sensorValY = 0;  
 
int oldX = 0;
int oldY = 0;

/**Rotor Vars**/
// Rotary Encoder Input efintions
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

unsigned long AlastButtonPress = 0;
unsigned long BlastButtonPress = 0;
//

   
void setup() {

  delay(5000);  //you want this delay. reason: tl;dr
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
    SD.remove("datalog.txt");
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
sensors();
 // listen for incoming clients
  EthernetClient client = server.available();
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
  }
void sensors(){
    int sensorValX = analogRead(8);
  int sensorValY = analogRead(9);
  String dataString = "";
  int threshhold =3;
  int xVariance = abs(sensorValX-oldX);
  int yVariance = abs(sensorValY-oldY);

if (xVariance >=threshhold && yVariance >= threshhold ||xVariance >=threshhold || yVariance >=threshhold){
//if sensor data is greater than threshold, write data to the sd card, then close the file
    dataString += String(sensorValX);
    dataString += ",";
    dataString += String(sensorValY);

   dataFile = SD.open("datalog.txt", FILE_WRITE);
    // if the file is available, write to it:
   if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      
      //clear dataString
      dataString="";
  } else {
    // if the file isn't open, pop up an error:
    Serial.println("error opening datalog.txt");
  }
dataFile.close();
  
}else{
  ;
  }

  }

void pageWrite(EthernetClient client){
          startPage(client);   
          svgHeader(client);           
          polyLineBegin(client);
          polyLineEnd(client);
          svgFooter(client);       
          endPage(client);  
  
  }


void startPage(EthernetClient client){
     // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>"); 
  }

 void endPage(EthernetClient client){
  client.println("</body>");
  client.println("</html>");
  }

  
void polyLineBegin(EthernetClient client){
  client.print("<polyline points=\""); //x,y points go here
  //read data from SD card
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
  
void polyLineEnd(EthernetClient client){
   client.println ("0, 0\" stroke=\"red\" fill=\"transparent\" stroke-width=\"5\"/>");  //a dirty hack to avoid dealing with 1 last trailing comma.
  }
 
void svgHeader(EthernetClient client){
client.println("<svg width=\"1023\" height=\"1023\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">"); 
  } 
  
void svgFooter(EthernetClient client){
client.println ("</svg>");
  }

void background(EthernetClient client){
//  client.println();

client.println("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
"<svg"
"xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:cc=\"http://creativecommons.org/ns#\" xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\""
"xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\""
"id=\"svg8\" version=\"1.1\" viewBox=\"0 0 227.35277 156.29323\""
"height=\"6.1532764in\""
"width=\"8.9508963in\""
"sodipodi:docname=\"etchasketchborder.svg\">"
"<defs"
"id=\"defs2\" />"
"<path"
"style=\"fill:#ff0000;stroke:#ff0000;stroke-width:1.32300019;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\""
"d=\"m 12.000819,0.66145838 c -6.2819637,0 -11.33936062,5.05739692 -11.33936062,11.33936062 V 144.29249 c 0,6.28197 5.05739692,11.33936 11.33936062,11.33936 H 215.35224 c 6.28197,0 11.33885,-5.05739 11.33885,-11.33936 V 12.000819 c 0,-6.2819637 -5.05688,-11.33936062 -11.33885,-11.33936062 z M 18.932696,12.696383 H 205.5854 c 5.02557,0 9.07128,4.04571 9.07128,9.071282 v 77.03923 c 0,5.025575 -4.04571,9.071285 -9.07128,9.071285 H 18.932696 c -5.025572,0 -9.0712819,-4.04571 -9.0712819,-9.071285 v -77.03923 c 0,-5.025572 4.0457099,-9.071282 9.0712819,-9.071282 z\""
"/>"
"<ellipse"
"style=\"fill:#ffffff;fill-opacity:1;stroke:#ff0000;stroke-width:1.32300007;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\""
"cx=\"21.16666\""
"cy=\"130.59085\""
"rx=\"10.583334\""
"ry=\"9.0714283\" />"
"<ellipse"
"style=\"fill:#ffffff;fill-opacity:1;stroke:#ff0000;stroke-width:1.32300007;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\""
"cx=\"200.62982\""
"cy=\"131.57361\""
"rx=\"10.583334\""
"ry=\"9.0714283\" />"
"</svg>");
  }  

 void rotorA(EthernetClient client){
 // Read the current state of CLK
  AcurrentStateCLK = digitalRead(CLKA);

  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (AcurrentStateCLK != AlastStateCLK  && AcurrentStateCLK == 1){

    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DTA) != AcurrentStateCLK) {
      Acounter --;
      AcurrentDir ="CCW";
    } else {
      // Encoder is rotating CW so increment
      Acounter ++;
      AcurrentDir ="CW";
    }
    Serial.print("A Direction: ");
    Serial.print(AcurrentDir);
    Serial.print(" | Counter: ");
    Serial.println(Acounter);
  }
  int AbtnState = digitalRead(SWA);

  //If we detect LOW signal, button is pressed
  if (AbtnState == LOW) {
    //if 50ms have passed since last LOW pulse, it means that the
    //button has been pressed, released and pressed again
    if (millis() - AlastButtonPress > 500) {
      Serial.println("Button A pressed!");
    }
  AlastButtonPress = millis();
  }
  // Remember last CLK state
  AlastStateCLK = AcurrentStateCLK;
//endA


  }
void rotorB(EthernetClient client){
  
  BcurrentStateCLK = digitalRead(CLKB);
  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (BcurrentStateCLK != BlastStateCLK  && BcurrentStateCLK == 1){

    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DTB) != BcurrentStateCLK) {
      Bcounter --;
      BcurrentDir ="CCW";
    } else {
      // Encoder is rotating CW so increment
      Bcounter ++;
      BcurrentDir ="CW";
    }

    Serial.print("B Direction: ");
    Serial.print(BcurrentDir);
    Serial.print(" | Counter: ");
    Serial.println(Bcounter);
  }  
  BlastStateCLK = BcurrentStateCLK;
  // Read the button state
  int BbtnState = digitalRead(SWB);
    if (BbtnState == LOW) {
    //if 50ms have passed since last LOW pulse, it means that the
    //button has been pressed, released and pressed again
    if (millis() - BlastButtonPress > 50) {
      Serial.println("Button B pressed!");
    }
  
 // Remember last button press event
    BlastButtonPress = millis();
  }
//endB
}
void rotorSense(EthernetClient client){
  rotorA(client);
  rotorB(client);
  }
