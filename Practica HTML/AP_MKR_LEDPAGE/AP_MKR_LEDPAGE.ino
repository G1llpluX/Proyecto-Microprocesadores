
/****************************************************************************
Web Server in an Access Point for Arduino MKR1000                           *
Reference :                                                                 *
https://docs.arduino.cc/tutorials/mkr-1000-wifi/mkr-1000-web-server-ap-mode *
By: Karl Söderby                                                            *
Adapted and Modified by : Luis Ariel González for an Introductory Course    *
of Microcontrollers                                                         *
                                                                            *
Example of a simple form in HTML                                            *
*****************************************************************************/

#include <SPI.h>
#include <WiFi101.h>


// Global Variables for the WiFi connection
char ssid[] = "ARIEL-MKR";        // your network SSID (name)
int status = WL_IDLE_STATUS;
WiFiServer server(80);

// Global Variables to handle the incomming data from the Client
WiFiClient client;
String currentLine = "";                // make a String to hold incoming data from the client

//Global Variables for the Arduino Hardware 
int led =  LED_BUILTIN;
bool ledState = LOW;
bool blinkMode= false; // if blink mode then blink
int blinkTimer=0;
unsigned long lastBlinkTimer;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // Intialize the Web Access Point
  Serial.println("Access Point Web Server");
  // print the network name (SSID);
  Serial.print("Creating access point named: ");
  Serial.println(ssid);
  // Create open network. Change this line if you want to create an WEP network:
  status = WiFi.beginAP(ssid);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    // don't continue
    while (true);
  }
  // Intialize Arduino Pins
 pinMode(led, OUTPUT);      // set the LED pin mode
  // wait 10 seconds for connection:
  delay(10000);
  // start the web server on port 80
  server.begin();
  // you're connected now, so print out the status
  printWiFiStatus();
}

void handleBlink(){
  unsigned long now=millis();
  if ((now - lastBlinkTimer) >= blinkTimer){
    lastBlinkTimer=now;
    ledState = !ledState;
    digitalWrite(led,ledState);
  }

}
void loop() {
  checkConnectionStatus(); 
  handleClient();
  if (blinkMode) handleBlink();
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}
void checkConnectionStatus(){
// compare the previous status to the current status
  if (status != WiFi.status()) {
    // it has changed update the variable
    status = WiFi.status();

    if (status == WL_AP_CONNECTED) {
      // a device has connected to the AP
      Serial.println("Device connected to AP");
    } else {
      // a device has disconnected from the AP, and we are back in listening mode
      Serial.println("Device disconnected from AP");
    }
  }
}
void handleClient(){
  client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    currentLine = "";                       // initialize currentLine for incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            sendResponse();
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
        handleCurrentLine();
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

void sendResponse(){
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();// maybe no hace falta
  // the content of the HTTP response follows the header:
  client.println("<head>");
  client.println("<title>MKR LED</title>");
  client.println("</head>");
  client.println("<body>");
  client.println("<h1>ARDUINO MKR LED CONTROL PAGE</h1>");
  client.println("<br><br>");
  client.print("<form action=\"/get\">");
  client.print("<input type=\"hidden\" id=\"ledON\" name=\"ledStatus\" value=\"H\">");
  client.print("<input type=\"submit\" value=\"LED ON\">");
  client.print("</form><br>");
  client.print("<form action=\"/get\">");
  client.print("<input type=\"hidden\" id=\"ledOFF\" name=\"ledStatus\" value=\"L\">");
  client.print("<input type=\"submit\" value=\"LED OFF\"><br>");
  client.print("</form><br>");
  client.print("<form>");
  client.print("<label for=\"blink\">Blink (between 0 and 5) seconds:</label>");
  client.print("<input type=\"range\" id=\"blink\" name=\"blink\" min=\"0\" max=\"5\">");
  client.print("<input type=\"submit\" value=\"BLINK\">");
  client.print("</form>");
  client.println("</body>");
  // The HTTP response ends with another blank line:
  client.println();
}

void handleCurrentLine(){
  // Check to see if the client request was "GET /H" or "GET /L":
  if (currentLine.endsWith("?ledStatus=H")) {
    digitalWrite(led, HIGH);               // GET /H turns the LED on
    ledState=HIGH;
    blinkMode=false;
    blinkTimer=0;
  }
  if (currentLine.endsWith("?ledStatus=L")) {
    digitalWrite(led, LOW);                // GET /L turns the LED off
    ledState=LOW;
    blinkMode=false;
    blinkTimer=0;
  }
  int len = currentLine.length();
  String testBlink = currentLine.substring(1,len-1);
  if (testBlink.endsWith("?blink=")){
    blinkMode=true;
    Serial.println();
    Serial.println("IS A BLINK");
    String blinkValue = currentLine.substring(len-1,len);
    Serial.print("VALUE=");
    Serial.println(blinkValue);
    blinkTimer= 1000*blinkValue.toInt();
    Serial.print("blinkTimer=");
    Serial.println(blinkTimer);
  }
}