#define LOG_LEVEL LOG_LEVEL_VERBOSE

void setupLog();

void logVerbose(const char* message);
void logVerboseln(const char* message);

void logInfo(const char* message);
void logInfoln(const char* message);

void logError(const char* message);
void logErrorln(const char* message);