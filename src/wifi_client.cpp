#include "wifi_client.h"

IPAddress ip;
IPAddress subnet;
IPAddress gateway;

void setupWiFi() {
  #if (WIFI == 1)
  logInfoln("[WiFi] Setting up");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  for (int n = 0; n < 3; n++) {
    for (int i = 0; i < 3; i++) {
      if (WiFi.status() == WL_CONNECTED) {
        ip = WiFi.localIP();
        subnet = WiFi.subnetMask();
        gateway = WiFi.gatewayIP();
        return;
      }
      //waiting for connection to be up
      delay(1000);
      logVerboseln("[WiFi] No connection -> waiting");
    }
    // trying to connect again
    WiFi.disconnect();
    logInfoln("[WiFi] No connection -> retrying");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
  // giving up. Configuration might be at fault.
  if (WiFi.status() != WL_CONNECTED) {
    logFatalln("[WiFi] No connection -> giving up");
    return;
  }
  // wifi reconnect, maybe? no idea how to do it yet
  #endif
}

void printWiFiInfo() {
  //IP needs to be converted to hex and then reversed and then byte wise transformed to dezimal to be readable. e.g
  //1387440320 to hex 0x52B2A8C0 -> 52 B2 A8 C0 reverse to C0 A8 B2 52 to dec 192.168.178.82
  //255 255 255 0
  //192.168.178.236
  #if (WIFI == 1)
  logInfoln(("[WiFi] IP: "+String(ip)+"; Subnet: "+String(subnet)+"; Gateway: "+String(gateway)).c_str());
  #endif
}
