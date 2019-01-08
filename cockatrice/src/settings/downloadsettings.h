#ifndef COCKATRICE_DOWNLOADSETTINGS_H
#define COCKATRICE_DOWNLOADSETTINGS_H

#include "settingsmanager.h"
#include <QObject>

class DownloadSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

public:
    explicit DownloadSettings(const QString &, QObject *);

    QStringList getAllURLs();
    QString getDownloadUrlAt(int);
    void setDownloadUrlAt(int, const QString &);
    int getCount();
    void clear();

private:
    QStringList downloadURLs;

private:
    void populateDefaultURLs();
};

#endif // COCKATRICE_DOWNLOADSETTINGS_H
