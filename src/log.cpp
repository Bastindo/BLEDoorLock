#include "log.h"

void printTimestamp(Print* _logOutput) {
    const unsigned long MSECS_PER_SEC = 1000;
    const unsigned long SECS_PER_MIN = 60;
    const unsigned long SECS_PER_HOUR = 3600;
    const unsigned long SECS_PER_DAY = 86400;

    const unsigned long msecs = millis();
    const unsigned long secs = msecs / MSECS_PER_SEC;

    const unsigned long MilliSeconds = msecs % MSECS_PER_SEC;
    const unsigned long Seconds = secs % SECS_PER_MIN;
    const unsigned long Minutes = (secs / SECS_PER_MIN) % SECS_PER_MIN;
    const unsigned long Hours = (secs % SECS_PER_DAY) / SECS_PER_HOUR;

    char timestamp[20];
    sprintf(timestamp, "(%02d:%02d:%02d.%03d | ", Hours, Minutes, Seconds, MilliSeconds);
    _logOutput->print(timestamp);
}

void printLogLevel(Print* _logOutput, int logLevel) {
    switch (logLevel) {
        default:
        case 0:
            _logOutput->print("SILENT) ");
            break;
        case 1:
            _logOutput->print("FATAL) ");
            break;
        case 2:
            _logOutput->print("AUTH) ");
            break;
        case 3:
            _logOutput->print("WARNING) ");
            break;
        case 4:
            _logOutput->print("INFO) ");
            break;
        case 5:
            _logOutput->print("TRACE) ");
            break;
        case 6:
            _logOutput->print("VERBOSE) ");
            break;
    }
}

// prefix for log messages
void printPrefix(Print* _logOutput, int logLevel) {
    printTimestamp(_logOutput);
    printLogLevel(_logOutput, logLevel);
}

void setupFileLog() {
    if (!LittleFS.begin(true)) {
        logFatalln("[UserAuth::setupUserAuth] Failed to mount file system");
        return;
    }

    if (!LittleFS.exists("/log.csv")) {
        File logs = LittleFS.open("/log.csv", "w");
        logs.close();
    }
}

void logUserOpenEvent(const UserOpenEvent* event) {
    File logs = LittleFS.open("/log.csv", "a");
    if (!logs) {
        logFatalln("[logUserOpenEvent] Failed to open log file");
        return;
    }

    logs.print(event->unixTime);
    logs.print(",");
    logs.print(event->username.c_str());
    logs.print(",");
    logs.print(event->state);
    logs.println();
    logs.close();
}

void setupLog() {
    Log.setPrefix(printPrefix);
    Log.begin(LOG_LEVEL, &Serial);
    Log.setShowLevel(false);  // Do not show loglevel since it is already shown in prefix
}

void logVerbose(const char* message) {
    Log.verbose("%s", message);
}

void logVerboseln(const char* message) {
    Log.verbose("%s\r\n", message);
}

void logInfo(const char* message) {
    Log.notice("%s", message);
}

void logInfoln(const char* message) {
    Log.notice("%s\r\n", message);
}

void logWarn(const char* message) {
    Log.warning("%s", message);
}

void logWarnln(const char* message) {
    Log.warning("%s\r\n", message);
}

void logAuth(const char* message) {
    Log.error("%s", message);
}

void logAuthln(const char* message) {
    Log.error("%s\r\n", message);
}

void logFatal(const char* message) {
    Log.fatal("%s", message);
}

void logFatalln(const char* message) {
    Log.fatal("%s\r\n", message);
}