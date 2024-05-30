#ifndef LOG_H
#define LOG_H

#include <ArduinoLog.h>

#include "config.h"

void setupLog();

void logVerbose(const char* message);
void logVerboseln(const char* message);

void logInfo(const char* message);
void logInfoln(const char* message);

void logError(const char* message);
void logErrorln(const char* message);

#endif