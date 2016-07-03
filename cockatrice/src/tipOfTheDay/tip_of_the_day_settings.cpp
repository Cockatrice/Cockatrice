#include "tip_of_the_day_settings.h"


TipOfTheDaySettings::TipOfTheDaySettings(const QString &settingPath, QObject *parent) : SettingsManager(
        settingPath + "tipOfTheDay.ini",
        parent
)
{ }
