#include <MitsuAc.h>
#include <ESP8266WiFi.h>
#include <String.h>

WiFiServer server(80);
MitsuAc ma = MitsuAc();

void setup() {
  server.begin();
}

void loop() {
  WiFi.begin("<YOUR SSID>", "<YOUR WIFI PASSWORD>");
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }
  
  WiFiClient client = server.available();
  if (!client) { 
    return; 
  }

  while(!client.available()){
    delay(1);
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE HTML><html><p>Room Temp: ");
  client.println(ma.getRoomTempJson());
  client.println("<p>Settings: ");
  client.println(ma.getSettingsJson());
  client.println("</html>");
  delay(1);

}
