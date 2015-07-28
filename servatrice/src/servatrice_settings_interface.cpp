#include "servatrice.h"
#include "settingscache.h"
#include <QDebug>

bool Servatrice_SettingsInterface::getRequireRegistration()
{
    return settingsCache->value("authentication/regonly", 0).toBool();
}

