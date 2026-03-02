#include "message_settings.h"

MessageSettings::MessageSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "messages.ini", "messages", QString(), parent)
{
}

QString MessageSettings::getMessageAt(int index)
{
    return getValue(QString("msg%1").arg(index)).toString();
}

int MessageSettings::getCount()
{
    return getValue("count").toInt();
}

void MessageSettings::setCount(int count)
{
    setValue(count, "count");
}

void MessageSettings::setMessageAt(int index, QString message)
{
    setValue(message, QString("msg%1").arg(index));
}
