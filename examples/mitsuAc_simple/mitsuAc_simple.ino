#include <MitsuAc.h>
#include <ESP8266WiFi.h>

WiFiServer server(80);
MitsuAc ac = MitsuAc();

IPAddress ip(192, 168, 1, 190);
IPAddress gateway(192, 168, 1, 254);
IPAddress subnet(255, 255, 255, 0);

void setup() {
  WiFi.config(ip, gateway, subnet);
  WiFi.begin("xxx", "xxx");  
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }  
  server.begin();

  ac.connect(&Serial);
}

void loop() {  
  ac.monitor();

  WiFiClient client = server.available();
  if (client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");
    client.println("<!DOCTYPE HTML><html><p>Room Temp: ");
    client.println(ac.getRoomTempJson().c_str());
    client.println("<p>Settings: ");
    client.println(ac.getSettingsJson().c_str());
    client.println(buf.c_str());
    client.println("</html>");
    client.stop();
  }
}
