#ifndef TCP_H
#define TCP_H

#include <WiFi.h>

#include "config.h"
#include "log.h"
#include "wifi_client.h"

void tcpWrite(const char *message);

#endif