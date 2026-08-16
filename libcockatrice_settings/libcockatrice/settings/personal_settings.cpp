#include "personal_settings.h"

PersonalSettings::PersonalSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "personal.ini", "personal", QString(), parent)
{
}

QString PersonalSettings::getLang() const
{
    return getValue("lang", QString(), QString(), QString()).toString();
}

bool PersonalSettings::getShowTipsOnStartup() const
{
    return getValue("showTips", "tipOfDay", QString(), true).toBool();
}

QList<int> PersonalSettings::getSeenTips() const
{
    QList<int> tips;
    auto tipValues = getValue("seenTips", "tipOfDay", QString()).toList();
    for (const auto &tipNumber : tipValues) {
        tips.append(tipNumber.toInt());
    }
    return tips;
}

void PersonalSettings::setLang(const QString &_lang)
{
    setValue(_lang, "lang");
    emit langChanged();
}

void PersonalSettings::setShowTipsOnStartup(bool _showTipsOnStartup)
{
    setValue(_showTipsOnStartup, "showTips", "tipOfDay");
}

void PersonalSettings::setSeenTips(const QList<int> &_seenTips)
{
    QList<QVariant> storedTipList;
    for (auto tipNumber : _seenTips) {
        storedTipList.append(tipNumber);
    }
    setValue(QVariant::fromValue(storedTipList), "seenTips", "tipOfDay");
}
