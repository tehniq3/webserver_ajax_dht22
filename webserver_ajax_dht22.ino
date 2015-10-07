/*--------------------------------------------------------------
  Program:      eth_websrv_AJAX_IN
http://startingelectronics.org/tutorials/arduino/ethernet-shield-web-server-tutorial/AJAX-read-switches-analog/
  Description:  Uses Ajax to update the state of two switches
                and an analog input on a web page. The Arduino
                web server hosts the web page.
                Does not use the SD card.
  
  Hardware:     Arduino Uno and official Arduino Ethernet
                shield. Should work with other Arduinos and
                compatible Ethernet shields.
                
  Software:     Developed using Arduino 1.0.3 software
                Should be compatible with Arduino 1.0 +
  
  References:   - WebServer example by David A. Mellis and 
                  modified by Tom Igoe
                - Ethernet library documentation:
                  http://arduino.cc/en/Reference/Ethernet
                - Learning PHP, MySQL & JavaScript by
                  Robin Nixon, O'Reilly publishers

  Date:         20 February 2013
 
  Author:       W.A. Smith, http://startingelectronics.org

 Modified by Nicu Florica aka niq_ro
 visit http://nicuflorica.blogspot.ro/
 or http://arduinotehniq.blogspot.com
 or http://www.tehnic.go.ro
 Weather Station = Arduino webserver using Ajax with W5100 Ethernet Shield
--------------------------------------------------------------*/

#include <SPI.h>
#include <Ethernet.h>

// MAC address from Ethernet shield sticker under board
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//IPAddress ip(10, 0, 0, 20); // IP address, may need to change depending on network
IPAddress ip(192, 168, 0, 200);
EthernetServer server(80);  // create a server at port 80

String HTTP_req;            // stores the HTTP request

float t,h;        // value for temperature and humidity
#include "DHT.h"
#define DHTPIN A2     // what pin we're connected to
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor
DHT dht(DHTPIN, DHTTYPE);

void setup()
{
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients
    Serial.begin(9600);       // for diagnostics

    dht.begin();              // initialize DHT sensor
}

void loop()
{
    EthernetClient client = server.available();  // try to get client

    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                HTTP_req += c;  // save the HTTP request 1 char at a time
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: keep-alive");
                    client.println();
                    // AJAX request for switch state
                    if (HTTP_req.indexOf("ajax_switch") > -1) {
                        // read switch state and analog input
                        GetAjaxData(client);
                    }
                    else {  // HTTP request for web page
                        // send web page - contains JavaScript with AJAX calls
                        client.println("<!DOCTYPE html>");
                        client.println("<html>");
                        client.println("<head>");
                        client.println("<title>Arduino Web Page</title>");
     client.println("<meta name='viewport' content='width=200px'/>");
     client.println("<div style='position:absolute;width:200px;height:200px;top:50%;left:50%;margin:-100px 0 0 -100px'>");
     client.println("</div>");
                        client.println("<script>");
                        client.println("function GetSwitchAnalogData() {");
                        client.println("nocache = \"&nocache=\" + Math.random() * 1000000;");
                        client.println("var request = new XMLHttpRequest();");
                        client.println("request.onreadystatechange = function() {");
                        client.println("if (this.readyState == 4) {");
                        client.println("if (this.status == 200) {");
                        client.println("if (this.responseText != null) {");
                        client.println("document.getElementById(\"sw_an_data\")\
                        .innerHTML = this.responseText;");
                        client.println("}}}}");
                        client.println(
                        "request.open(\"GET\", \"ajax_switch\" + nocache, true);");
                        client.println("request.send(null);");
                        client.println("setTimeout('GetSwitchAnalogData()', 3000);");
                        client.println("}");
                        client.println("</script>");
                        client.println("</head>");
                        client.println("<body onload=\"GetSwitchAnalogData()\">");
                        client.println("<body style=background:#F0FFFF>");
                        client.print("<center>");
                        client.println("<h1>Arduino AJAX - DHT22</h1>");
                        client.println("<div id=\"sw_an_data\">");
                        client.println("</div>");
                        client.println("Visit <a href=http://www.tehnic.go.ro target=blank>http://www.tehnic.go.ro</a> & <a href=http://nicuflorica.blogspot.ro target=blank>http://nicuflorica.blogspot.ro/</a> for more projects!</p>");  
                        client.println("</body>");
                        client.println("</html>");
                    }
                    // display received HTTP request on serial port
                    Serial.print(HTTP_req);
                    HTTP_req = "";            // finished with request, empty string
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)
}

// send the state of the switch to the web browser
void GetAjaxData(EthernetClient cl)
{
    h = dht.readHumidity();
    t = dht.readTemperature();
    delay(2000);
    cl.println("<font color=red>"); 
    cl.print("<h2>");
    cl.print("<p>Temperature is: ");
    cl.print(t);
    cl.print("<sup>0</sup>C ");
    cl.println("</p>");
    cl.print("<p>Humidity is: ");
    cl.print(h);
    cl.print("%RH ");
    cl.println("</p>");
    cl.println("</font>"); 

}
