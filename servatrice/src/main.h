#ifndef MAIN_H
#define MAIN_H

class ServerLogger;
class QThread;
class SettingsCache;

extern ServerLogger *logger;
extern QThread *loggerThread;
extern SettingsCache *settingsCache;

#endif
