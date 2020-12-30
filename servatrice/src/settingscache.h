#ifndef SERVATRICE_SETTINGSCACHE_H
#define SERVATRICE_SETTINGSCACHE_H

#include <QList>
#include <QRegExp>
#include <QSettings>
#include <QString>

class SettingsCache : public QSettings
{
    Q_OBJECT
private:
    bool isPortableBuild;

public:
    SettingsCache(const QString &fileName = "servatrice.ini",
                  QSettings::Format format = QSettings::IniFormat,
                  QObject *parent = 0);
    static QString guessConfigurationPath(QString &specificPath);
    QList<QRegExp> disallowedRegExp;
    bool getIsPortableBuild() const
    {
        return isPortableBuild;
    }
};

extern SettingsCache *settingsCache;

#endif
