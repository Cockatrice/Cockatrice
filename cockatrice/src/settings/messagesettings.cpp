#include "messagesettings.h"

MessageSettings::MessageSettings(QString settingPath, QObject *parent)
    : SettingsManager(settingPath+"messages.ini",parent)
{
}

QString MessageSettings::getMessageAt(int index)
{
    return getValue(QString("msg%1").arg(index),"messages").toString();
}

int MessageSettings::getCount()
{
    return getValue("count", "messages").toInt();
}

void MessageSettings::setCount(int count)
{
    setValue(count,"count","messages");
}

void MessageSettings::setMessageAt(int index, QString message)
{
    setValue(message,QString("msg%1").arg(index),"messages");
}
