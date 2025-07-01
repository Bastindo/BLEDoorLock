#include "relay.h"

void setupRelay() {
  pinMode(RELAY_PIN, OUTPUT);
  relayClose();
}

void setRelayState(bool state) { digitalWrite(RELAY_PIN, state ? HIGH : LOW); }

int getRelayState() { return digitalRead(RELAY_PIN); }

void relayOpen() {
  setRelayState(true);
  Serial.println("Relay Opened");
  digitalWrite(RELAY_PIN, HIGH);
  Serial.println("Relay State: " + String(getRelayState()));
}

void relayClose() {
  setRelayState(false);
  Serial.println("Relay Closed");
  digitalWrite(RELAY_PIN, LOW);
  Serial.println("Relay State: " + String(getRelayState()));
}