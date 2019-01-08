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
    QList<QString> downloadURLs;
};

#endif // COCKATRICE_DOWNLOADSETTINGS_H
