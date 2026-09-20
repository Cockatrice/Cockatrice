#ifndef PATHS_SETTINGS_H
#define PATHS_SETTINGS_H

#include "settings_manager.h"

#include <libcockatrice/interfaces/interface_paths_settings_provider.h>

class PathsSettings : public SettingsManager, public IPathsSettingsProvider
{
    Q_OBJECT
    friend class SettingsCache;

public:
    [[nodiscard]] QString getDeckPath() const override;
    [[nodiscard]] QString getFiltersPath() const override;
    [[nodiscard]] QString getReplaysPath() const override;
    [[nodiscard]] QString getPicsPath() const override;
    [[nodiscard]] QString getCustomPicsPath() const override;
    [[nodiscard]] QString getThemesPath() const override;
    [[nodiscard]] QString getCardDatabasePath() const override;
    [[nodiscard]] QString getCustomCardDatabasePath() const override;
    [[nodiscard]] QString getTokenDatabasePath() const override;
    [[nodiscard]] QString getSpoilerCardDatabasePath() const override;
    [[nodiscard]] QString getRedirectCachePath() const override;

    void setDeckPath(const QString &_deckPath);
    void setFiltersPath(const QString &_filtersPath);
    void setReplaysPath(const QString &_replaysPath);
    void setPicsPath(const QString &_picsPath);
    void setCustomPicsPath(const QString &_customPicsPath);
    void setThemesPath(const QString &_themesPath);
    void setCardDatabasePath(const QString &_cardDatabasePath);
    void setCustomCardDatabasePath(const QString &_customCardDatabasePath);
    void setTokenDatabasePath(const QString &_tokenDatabasePath);
    void setSpoilerDatabasePath(const QString &_spoilerDatabasePath);

signals:
    void cardDatabasePathChanged();
    void picsPathChanged();
    void themeChanged();

private:
    explicit PathsSettings(const QString &settingPath, QObject *parent = nullptr);
    PathsSettings(const PathsSettings & /*other*/);
};

#endif // PATHS_SETTINGS_H
