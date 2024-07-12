#include <Arduino.h>

#include "config.h"

#include "ble_server.h"
#include "cmd.h"
#include "log.h"
#include "opendoor.h"
#include "user_auth.h"

void setup() {
  Serial.begin(115200);
  #if DEBUG_MODE == 1
    while(!Serial && !Serial.available()){}
  #endif

  setupLog();

  logInfoln("Starting up...");
  uint32_t freq = getCpuFrequencyMhz();
  String freqString = "CPU Frequency: " + String(freq) + "MHz\n";
  logInfo(freqString.c_str());

  setupOpener();
  setupBLE();
  setupUserAuth();
  setupCmd();
}

void loop() {
  cmdLoop(DEBUG_MODE);
}