#ifndef CHAT_SETTINGS_H
#define CHAT_SETTINGS_H

#include "settings_manager.h"

#include <libcockatrice/interfaces/interface_chat_settings_provider.h>

class ChatSettings : public SettingsManager, public IChatSettingsProvider
{
    Q_OBJECT
    friend class SettingsCache;

public:
    [[nodiscard]] bool getChatMention() const override;
    [[nodiscard]] bool getChatMentionCompleter() const override;
    [[nodiscard]] QString getChatMentionColor() const override;
    [[nodiscard]] QString getChatHighlightColor() const override;
    [[nodiscard]] bool getChatMentionForeground() const override;
    [[nodiscard]] bool getChatHighlightForeground() const override;
    [[nodiscard]] bool getIgnoreUnregisteredUsers() const override;
    [[nodiscard]] bool getIgnoreUnregisteredUserMessages() const override;
    [[nodiscard]] bool getIgnoreNonBuddyUserMessages() const override;
    [[nodiscard]] bool getShowMessagePopup() const override;
    [[nodiscard]] bool getShowMentionPopup() const override;
    [[nodiscard]] bool getRoomHistory() const override;
    [[nodiscard]] QString getHighlightWords() const override;

    void setChatMention(bool _chatMention);
    void setChatMentionCompleter(bool _chatMentionCompleter);
    void setChatMentionColor(const QString &_chatMentionColor);
    void setChatHighlightColor(const QString &_chatHighlightColor);
    void setChatMentionForeground(bool _chatMentionForeground);
    void setChatHighlightForeground(bool _chatHighlightForeground);
    void setIgnoreUnregisteredUsers(bool _ignoreUnregisteredUsers);
    void setIgnoreUnregisteredUserMessages(bool _ignoreUnregisteredUserMessages);
    void setIgnoreNonBuddyUserMessages(bool _ignoreNonBuddyUserMessages);
    void setShowMessagePopups(bool _showMessagePopups);
    void setShowMentionPopups(bool _showMentionPopups);
    void setRoomHistory(bool _roomHistory);
    void setHighlightWords(const QString &_highlightWords);

signals:
    void chatMentionCompleterChanged();

#ifdef SETTINGS_UNIT_TEST
public:
#else
private:
#endif
    explicit ChatSettings(const QString &settingPath, QObject *parent = nullptr);

private:
    ChatSettings(const ChatSettings & /*other*/);
};

#endif // CHAT_SETTINGS_H
