#include "opendoor.h"

void doorOpen() {
  servoOpen();
  relayOpen();
}

void doorClose() {
  servoClose();
  relayClose();
}

hw_timer_t *lockTimer = NULL;
LockState lockState = LOCKED;

void IRAM_ATTR lockTimerISR() {
  setLockState(LOCKED);
  Serial.println("Lock timer expired");
}

void setupLockTimer() {
  Serial.println("Setting up lock timer");
  // Short Lock Timer
  lockTimer = timerBegin(0, getCpuFrequencyMhz() * 1000, true);
  timerAttachInterrupt(lockTimer, &lockTimerISR, true);
  timerAlarmWrite(lockTimer, SHORT_UNLOCK_TIME, false);
}

void setupOpener() {
  setupServo();
  setupRelay();
  setupLockTimer();
}

LockState getLockState() { return lockState; }

void setLockStateByUser(LockState state, std::string username) {
  UserOpenEvent event;
  event.unixTime = time(nullptr);
  event.username = username;
  event.state = state;
  logUserOpenEvent(&event);
  setLockState(state);
}

void setLockState(LockState state) {
  if (state == SHORT_UNLOCK) {
    timerWrite(lockTimer, 0);
    timerAlarmEnable(lockTimer);
  } else if (state == UNLOCKED) {
    timerAlarmDisable(lockTimer);
  }
  lockState = state;
  setLockStateFromBLE(state);
  if (state == LOCKED) {
    doorClose();
  } else {
    doorOpen();
  }
}
