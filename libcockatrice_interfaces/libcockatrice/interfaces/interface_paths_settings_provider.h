#ifndef COCKATRICE_INTERFACE_PATHS_SETTINGS_PROVIDER_H
#define COCKATRICE_INTERFACE_PATHS_SETTINGS_PROVIDER_H

#include <QString>

class IPathsSettingsProvider
{
public:
    virtual ~IPathsSettingsProvider() = default;

    [[nodiscard]] virtual QString getDeckPath() const = 0;
    [[nodiscard]] virtual QString getFiltersPath() const = 0;
    [[nodiscard]] virtual QString getReplaysPath() const = 0;
    [[nodiscard]] virtual QString getPicsPath() const = 0;
    [[nodiscard]] virtual QString getCustomPicsPath() const = 0;
    [[nodiscard]] virtual QString getThemesPath() const = 0;
    [[nodiscard]] virtual QString getCardDatabasePath() const = 0;
    [[nodiscard]] virtual QString getCustomCardDatabasePath() const = 0;
    [[nodiscard]] virtual QString getTokenDatabasePath() const = 0;
    [[nodiscard]] virtual QString getSpoilerCardDatabasePath() const = 0;
    [[nodiscard]] virtual QString getRedirectCachePath() const = 0;
};

#endif // COCKATRICE_INTERFACE_PATHS_SETTINGS_PROVIDER_H
