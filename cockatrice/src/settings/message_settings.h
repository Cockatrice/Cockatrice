/**
 * @file message_settings.h
 * @ingroup NetworkSettings
 * @brief TODO: Document this.
 */

#ifndef MESSAGESETTINGS_H
#define MESSAGESETTINGS_H

#include "settings_manager.h"

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
    void messageMacrosChanged();

public slots:

private:
    explicit MessageSettings(const QString &settingPath, QObject *parent = nullptr);
    MessageSettings(const MessageSettings & /*other*/);
};

#endif // MESSAGESETTINGS_H
