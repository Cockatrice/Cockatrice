#include "personal_settings.h"

#include "libcockatrice/settings/settings_manager.h"

#include <QList>
#include <QVariant>

class QObject;

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

PersonalSettings::CrashReportingMode PersonalSettings::getCrashReportingMode() const
{
    const int raw = getValue("crashReportingMode", QString(), QString(), 0).toInt();
    switch (raw) {
        case 1:
            return CrashReportingMode::AlwaysSend;
        case 2:
            return CrashReportingMode::NeverSend;
        case 0:
        default:
            // Clamp corrupt ini values back to Ask; the QSettings key string is unchanged (no migration).
            return CrashReportingMode::Ask;
    }
}

void PersonalSettings::setCrashReportingMode(CrashReportingMode _crashReportingMode)
{
    setValue(static_cast<int>(_crashReportingMode), "crashReportingMode");
    emit crashReportingModeChanged(_crashReportingMode);
}
