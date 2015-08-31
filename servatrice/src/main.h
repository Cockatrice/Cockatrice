#ifndef MAIN_H
#define MAIN_H

class ServerLogger;
class QThread;
class SettingsCache;
class SmtpClient;

extern ServerLogger *logger;
extern QThread *loggerThread;
extern SettingsCache *settingsCache;
extern SmtpClient *smtpClient;

#endif
