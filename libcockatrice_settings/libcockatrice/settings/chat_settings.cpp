#include "chat_settings.h"

ChatSettings::ChatSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "chat.ini", "chat", QString(), parent)
{
}

bool ChatSettings::getChatMention() const
{
    return getValue("mention", QString(), QString(), true).toBool();
}

bool ChatSettings::getChatMentionCompleter() const
{
    return getValue("mentionCompleter", QString(), QString(), true).toBool();
}

QString ChatSettings::getChatMentionColor() const
{
    return getValue("mentionColor", QString(), QString(), "A6120D").toString();
}

QString ChatSettings::getChatHighlightColor() const
{
    return getValue("highlightColor", QString(), QString(), "A6120D").toString();
}

bool ChatSettings::getChatMentionForeground() const
{
    return getValue("mentionForeground", QString(), QString(), true).toBool();
}

bool ChatSettings::getChatHighlightForeground() const
{
    return getValue("highlightForeground", QString(), QString(), true).toBool();
}

bool ChatSettings::getIgnoreUnregisteredUsers() const
{
    return getValue("ignoreUnregistered").toBool();
}

bool ChatSettings::getIgnoreUnregisteredUserMessages() const
{
    return getValue("ignoreUnregisteredMessages").toBool();
}

bool ChatSettings::getIgnoreNonBuddyUserMessages() const
{
    return getValue("ignoreNonBuddyMessages").toBool();
}

bool ChatSettings::getShowMessagePopup() const
{
    return getValue("showMessagePopups", QString(), QString(), true).toBool();
}

bool ChatSettings::getShowMentionPopup() const
{
    return getValue("showMentionPopups", QString(), QString(), true).toBool();
}

bool ChatSettings::getRoomHistory() const
{
    return getValue("roomHistory", QString(), QString(), true).toBool();
}

QString ChatSettings::getHighlightWords() const
{
    return getValue("highlightWords").toString();
}

void ChatSettings::setChatMention(bool _chatMention)
{
    setValue(_chatMention, "mention");
}

void ChatSettings::setChatMentionCompleter(bool _chatMentionCompleter)
{
    setValue(_chatMentionCompleter, "mentionCompleter");
    emit chatMentionCompleterChanged();
}

void ChatSettings::setChatMentionColor(const QString &_chatMentionColor)
{
    setValue(_chatMentionColor, "mentionColor");
}

void ChatSettings::setChatHighlightColor(const QString &_chatHighlightColor)
{
    setValue(_chatHighlightColor, "highlightColor");
}

void ChatSettings::setChatMentionForeground(bool _chatMentionForeground)
{
    setValue(_chatMentionForeground, "mentionForeground");
}

void ChatSettings::setChatHighlightForeground(bool _chatHighlightForeground)
{
    setValue(_chatHighlightForeground, "highlightForeground");
}

void ChatSettings::setIgnoreUnregisteredUsers(bool _ignoreUnregisteredUsers)
{
    setValue(_ignoreUnregisteredUsers, "ignoreUnregistered");
}

void ChatSettings::setIgnoreUnregisteredUserMessages(bool _ignoreUnregisteredUserMessages)
{
    setValue(_ignoreUnregisteredUserMessages, "ignoreUnregisteredMessages");
}

void ChatSettings::setIgnoreNonBuddyUserMessages(bool _ignoreNonBuddyUserMessages)
{
    setValue(_ignoreNonBuddyUserMessages, "ignoreNonBuddyMessages");
}

void ChatSettings::setShowMessagePopups(bool _showMessagePopups)
{
    setValue(_showMessagePopups, "showMessagePopups");
}

void ChatSettings::setShowMentionPopups(bool _showMentionPopups)
{
    setValue(_showMentionPopups, "showMentionPopups");
}

void ChatSettings::setRoomHistory(bool _roomHistory)
{
    setValue(_roomHistory, "roomHistory");
}

void ChatSettings::setHighlightWords(const QString &_highlightWords)
{
    setValue(_highlightWords, "highlightWords");
}
