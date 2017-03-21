#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QVariant>

class SettingsManager : public QObject
{
    Q_OBJECT
public:
    SettingsManager(QString settingPath, QObject *parent = 0);
    QVariant getValue(QString name, QString group = "", QString subGroup = "");

signals:

public slots:

protected:
    QSettings settings;
    void setValue(QVariant value, QString name, QString group = "", QString subGroup = "");
};

#endif // SETTINGSMANAGER_H
