#include "libcockatrice/rng/rng_abstract.h"
#include "main.h"
#include "server_logger.h"
#include "settingscache.h"
#include "smtpclient.h"

#include <QThread>

// Process-wide services the server sources reach through these pointers. They are defined here, not
// in main.cpp, so that every executable sharing servatrice_core sees the same instances.
RNG_Abstract *rng = nullptr;
ServerLogger *logger = nullptr;
QThread *loggerThread = nullptr;
SettingsCache *settingsCache = nullptr;
SmtpClient *smtpClient = nullptr;
