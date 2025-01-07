#ifndef LOG_H
#define LOG_H

#include <ArduinoLog.h>
#include <LittleFS.h>

#include "config.h"
#include "tcp.h"
struct UserOpenEvent {
    int64_t unixTime;
    std::string username;
    LockState state;
};

void setupFileLog();

void logUserOpenEvent(const UserOpenEvent* event);

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