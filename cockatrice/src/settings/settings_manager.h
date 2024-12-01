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
    explicit SettingsManager(QString settingPath, QObject *parent = nullptr);
    QVariant getValue(QString name, QString group = "", QString subGroup = "");

signals:

public slots:

protected:
    QSettings settings;
    void setValue(QVariant value, QString name, QString group = "", QString subGroup = "");
    void deleteValue(QString name, QString group = "", QString subGroup = "");
};

#endif // SETTINGSMANAGER_H
