#ifndef COCKATRICE_INTERFACE_CHAT_SETTINGS_PROVIDER_H
#define COCKATRICE_INTERFACE_CHAT_SETTINGS_PROVIDER_H

#include <QString>

class IChatSettingsProvider
{
public:
    virtual ~IChatSettingsProvider() = default;

    [[nodiscard]] virtual bool getChatMention() const = 0;
    [[nodiscard]] virtual bool getChatMentionCompleter() const = 0;
    [[nodiscard]] virtual QString getChatMentionColor() const = 0;
    [[nodiscard]] virtual QString getChatHighlightColor() const = 0;
    [[nodiscard]] virtual bool getChatMentionForeground() const = 0;
    [[nodiscard]] virtual bool getChatHighlightForeground() const = 0;
    [[nodiscard]] virtual bool getIgnoreUnregisteredUsers() const = 0;
    [[nodiscard]] virtual bool getIgnoreUnregisteredUserMessages() const = 0;
    [[nodiscard]] virtual bool getIgnoreNonBuddyUserMessages() const = 0;
    [[nodiscard]] virtual bool getShowMessagePopup() const = 0;
    [[nodiscard]] virtual bool getShowMentionPopup() const = 0;
    [[nodiscard]] virtual bool getRoomHistory() const = 0;
    [[nodiscard]] virtual QString getHighlightWords() const = 0;
};

#endif // COCKATRICE_INTERFACE_CHAT_SETTINGS_PROVIDER_H
