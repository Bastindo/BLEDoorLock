#ifndef LOG_H
#define LOG_H

#include <ArduinoLog.h>

#include "config.h"

void setupLog();

void logVerbose(const char* message);
void logVerboseln(const char* message);

void logInfo(const char* message);
void logInfoln(const char* message);

void logWarn(const char* message);
void logWarnln(const char* message);

void logAuth(const char* message);
void logAuthln(const char* message);

void logFatal(const char* message);
void logFatalln(const char* message);

#endif