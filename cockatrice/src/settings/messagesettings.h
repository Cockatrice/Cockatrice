#ifndef MESSAGESETTINGS_H
#define MESSAGESETTINGS_H

#include "settingsmanager.h"

class MessageSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

public:
    int getCount();
    QString getMessageAt(int index);

    void setCount(int count);
    void setMessageAt(int index, QString message);
signals:

public slots:

private:
    MessageSettings(QString settingPath, QObject *parent = 0);
    MessageSettings( const MessageSettings& /*other*/ );
    MessageSettings( MessageSettings& /*other*/ );
    MessageSettings( volatile const MessageSettings& /*other*/ );
    MessageSettings( volatile MessageSettings& /*other*/ );
};

#endif // MESSAGESETTINGS_H
