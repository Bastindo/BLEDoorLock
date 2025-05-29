#ifndef TCP_H
#define TCP_H 

#include "config.h"
#include "log.h"
#include <WiFi.h>
#include "wifi_client.h"

void tcpWrite(const char *message);

#endif