#ifndef COCKATRICE_INTERFACE_PERSONAL_SETTINGS_PROVIDER_H
#define COCKATRICE_INTERFACE_PERSONAL_SETTINGS_PROVIDER_H

#include <QDate>
#include <QList>
#include <QString>

class IPersonalSettingsProvider
{
public:
    virtual ~IPersonalSettingsProvider() = default;

    [[nodiscard]] virtual QString getLang() const = 0;
    [[nodiscard]] virtual bool getShowTipsOnStartup() const = 0;
    [[nodiscard]] virtual QList<int> getSeenTips() const = 0;
};

#endif // COCKATRICE_INTERFACE_PERSONAL_SETTINGS_PROVIDER_H
