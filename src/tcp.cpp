#include "tcp.h"
// Initialize the client library
WiFiClient client;
IPAddress server(HTTP_TARGET1, HTTP_TARGET2, HTTP_TARGET3, HTTP_TARGET4);

void tcpWrite(const char *message) {
  #if (WIFI == 1)
  if (WiFi.status() == WL_CONNECTED) {
    client.stop();
    if (client.connect(server, HTTP_PORT)) {
      Serial.println("[TCP] Connected and sending message");
      // send the message
      client.println(message);
      client.stop();
    } else {
      // if you couldn't make a connection:
      Serial.println("[TCP] Couldnt connect");
    }
  }
  #endif
}