#include "opendoor.h"

void setupOpener() {
    setupServo();
    setupRelay();
}

void doorOpen() {
    servoOpen();
    relayOpen();
}
void doorClose() {
    servoClose();
    relayClose();
}