#ifndef COCKATRICE_PATH_SETTINGS_H
#define COCKATRICE_PATH_SETTINGS_H

/**
 * @file path_settings.h
 * @ingroup Settings
 */
//! \todo Document this file.

#include "settings_manager.h"
#include <libcockatrice/interfaces/interface_card_database_path_provider.h>

#include <QLoggingCategory>
#include <QString>

inline Q_LOGGING_CATEGORY(PathSettingsLog, "path_settings");

/**
 * Owns every user-configurable filesystem path used by the client (decks, filters,
 * replays, themes, pictures, card databases, and the redirect cache).
 *
 * Backed by the "paths" group of the existing global.ini, so extracting this class out
 * of SettingsCache is a pure code-organization change: no stored user data moves.
 */
class PathSettings : public SettingsManager, public ICardDatabasePathProvider
{
    Q_OBJECT

    signals:
    void cardDatabasePathChanged();

public:
    explicit PathSettings(const QString &settingPath, QObject *parent = nullptr);

    [[nodiscard]] QString getDeckPath() const
    {
        return deckPath;
    }
    [[nodiscard]] QString getFiltersPath() const
    {
        return filtersPath;
    }
    [[nodiscard]] QString getReplaysPath() const
    {
        return replaysPath;
    }
    [[nodiscard]] QString getThemesPath() const
    {
        return themesPath;
    }
    [[nodiscard]] QString getPicsPath() const
    {
        return picsPath;
    }
    [[nodiscard]] QString getRedirectCachePath() const
    {
        return redirectCachePath;
    }
    [[nodiscard]] QString getCustomPicsPath() const
    {
        return customPicsPath;
    }
    [[nodiscard]] QString getCustomCardDatabasePath() const override
    {
        return customCardDatabasePath;
    }
    [[nodiscard]] QString getCardDatabasePath() const override
    {
        return cardDatabasePath;
    }
    [[nodiscard]] QString getSpoilerCardDatabasePath() const override
    {
        return spoilerDatabasePath;
    }
    [[nodiscard]] QString getTokenDatabasePath() const override
    {
        return tokenDatabasePath;
    }

    void setDeckPath(const QString &_deckPath);
    void setFiltersPath(const QString &_filtersPath);
    void setReplaysPath(const QString &_replaysPath);
    void setThemesPath(const QString &_themesPath);
    void setPicsPath(const QString &_picsPath);
    void setCustomCardDatabasePath(const QString &_customCardDatabasePath);
    void setCardDatabasePath(const QString &_cardDatabasePath);
    void setSpoilerDatabasePath(const QString &_spoilerDatabasePath);
    void setTokenDatabasePath(const QString &_tokenDatabasePath);

    /**
     * (Re)computes every path from defaults rooted at dataPath/cachePath, honoring any
     * user overrides already stored in settings. Must be called once after construction.
     */
    void load(const QString &dataPath, const QString &cachePath);

    /**
     * Clears all stored path overrides and reloads defaults rooted at dataPath/cachePath.
     */
    void reset(const QString &dataPath, const QString &cachePath);

private:
    QString deckPath, filtersPath, replaysPath, picsPath, redirectCachePath, customPicsPath, cardDatabasePath,
        customCardDatabasePath, themesPath, spoilerDatabasePath, tokenDatabasePath;

    [[nodiscard]] QString getSafeConfigPath(const QString &configEntry, const QString &defaultPath) const;
    [[nodiscard]] QString getSafeConfigFilePath(const QString &configEntry, const QString &defaultPath) const;

    void recomputeCustomPicsPath();
};


#endif //COCKATRICE_PATH_SETTINGS_H
