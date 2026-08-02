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
    return getValue("mentioncompleter", QString(), QString(), true).toBool();
}

QString ChatSettings::getChatMentionColor() const
{
    return getValue("mentioncolor", QString(), QString(), "A6120D").toString();
}

QString ChatSettings::getChatHighlightColor() const
{
    return getValue("highlightcolor", QString(), QString(), "A6120D").toString();
}

bool ChatSettings::getChatMentionForeground() const
{
    return getValue("mentionforeground", QString(), QString(), true).toBool();
}

bool ChatSettings::getChatHighlightForeground() const
{
    return getValue("highlightforeground", QString(), QString(), true).toBool();
}

bool ChatSettings::getIgnoreUnregisteredUsers() const
{
    return getValue("ignore_unregistered").toBool();
}

bool ChatSettings::getIgnoreUnregisteredUserMessages() const
{
    return getValue("ignore_unregistered_messages").toBool();
}

bool ChatSettings::getIgnoreNonBuddyUserMessages() const
{
    return getValue("ignore_nonbuddy_messages").toBool();
}

bool ChatSettings::getShowMessagePopup() const
{
    return getValue("showmessagepopups", QString(), QString(), true).toBool();
}

bool ChatSettings::getShowMentionPopup() const
{
    return getValue("showmentionpopups", QString(), QString(), true).toBool();
}

bool ChatSettings::getRoomHistory() const
{
    return getValue("roomhistory", QString(), QString(), true).toBool();
}

QString ChatSettings::getHighlightWords() const
{
    return getValue("highlightwords").toString();
}

void ChatSettings::setChatMention(bool _chatMention)
{
    setValue(_chatMention, "mention");
}

void ChatSettings::setChatMentionCompleter(bool _chatMentionCompleter)
{
    setValue(_chatMentionCompleter, "mentioncompleter");
    emit chatMentionCompleterChanged();
}

void ChatSettings::setChatMentionColor(const QString &_chatMentionColor)
{
    setValue(_chatMentionColor, "mentioncolor");
}

void ChatSettings::setChatHighlightColor(const QString &_chatHighlightColor)
{
    setValue(_chatHighlightColor, "highlightcolor");
}

void ChatSettings::setChatMentionForeground(bool _chatMentionForeground)
{
    setValue(_chatMentionForeground, "mentionforeground");
}

void ChatSettings::setChatHighlightForeground(bool _chatHighlightForeground)
{
    setValue(_chatHighlightForeground, "highlightforeground");
}

void ChatSettings::setIgnoreUnregisteredUsers(bool _ignoreUnregisteredUsers)
{
    setValue(_ignoreUnregisteredUsers, "ignore_unregistered");
}

void ChatSettings::setIgnoreUnregisteredUserMessages(bool _ignoreUnregisteredUserMessages)
{
    setValue(_ignoreUnregisteredUserMessages, "ignore_unregistered_messages");
}

void ChatSettings::setIgnoreNonBuddyUserMessages(bool _ignoreNonBuddyUserMessages)
{
    setValue(_ignoreNonBuddyUserMessages, "ignore_nonbuddy_messages");
}

void ChatSettings::setShowMessagePopups(bool _showMessagePopups)
{
    setValue(_showMessagePopups, "showmessagepopups");
}

void ChatSettings::setShowMentionPopups(bool _showMentionPopups)
{
    setValue(_showMentionPopups, "showmentionpopups");
}

void ChatSettings::setRoomHistory(bool _roomHistory)
{
    setValue(_roomHistory, "roomhistory");
}

void ChatSettings::setHighlightWords(const QString &_highlightWords)
{
    setValue(_highlightWords, "highlightwords");
}
