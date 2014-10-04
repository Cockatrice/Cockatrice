#ifndef SERVATRICE_SETTINGSCACHE_H
#define SERVATRICE_SETTINGSCACHE_H

#include <QSettings>
#include <QString>

class SettingsCache : public QSettings {
    Q_OBJECT
private:
    QSettings *settings;
public:
    SettingsCache(const QString & fileName="servatrice.ini", QSettings::Format format=QSettings::IniFormat, QObject * parent = 0);
    static QString guessConfigurationPath(QString & specificPath);
};

extern SettingsCache *settingsCache;

#endif
