#ifndef COCKATRICE_TIP_OF_THE_DAY_SETTINGS_H
#define COCKATRICE_TIP_OF_THE_DAY_SETTINGS_H

#include "../settings/settingsmanager.h"

class TipOfTheDaySettings : public SettingsManager
{
    Q_OBJECT
//private:
public:
    TipOfTheDaySettings(const QString &settingPath, QObject *parent);


public slots:

    void setShouldShowTipsOnStartup(int shouldShowTipsOnStartup);
};

#endif //COCKATRICE_TIP_OF_THE_DAY_SETTINGS_H
