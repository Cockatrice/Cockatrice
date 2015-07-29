#include "servatrice.h"
#include "settingscache.h"
#include <QDebug>

Servatrice_SettingsInterface::Servatrice_SettingsInterface(int _instanceId, Servatrice *_server)
{

}

Servatrice_SettingsInterface::~Servatrice_SettingsInterface()
{

}

bool Servatrice_SettingsInterface::getRequireRegistration()
{
    return settingsCache->value("authentication/regonly", 0).toBool();
}

