#ifndef PATHS_SETTINGS_H
#define PATHS_SETTINGS_H

#include "settings_manager.h"

#include <QString>
#include <QStringList>
#include <libcockatrice/interfaces/interface_paths_settings_provider.h>
#include <qtmetamacros.h>

class QObject;

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

    /**
     * @brief Returns the configured paths that resolve inside baseDir.
     *
     * Comparison is lexical (QDir::cleanPath); symlinks are not resolved, so a
     * path that only reaches baseDir through a symlink is not reported.
     */
    [[nodiscard]] QStringList pathsInsideDir(const QString &baseDir) const;

    /**
     * @brief Whether path resolves inside baseDir, as a single-path check.
     *
     * A path counts as inside when it equals baseDir or is nested below it;
     * a sibling sharing the name prefix (baseDir + "2") does not.
     */
    [[nodiscard]] static bool isInsideDir(const QString &path, const QString &baseDir);

    [[nodiscard]] bool getAppDirWarningAcknowledged() const;

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
    void setAppDirWarningAcknowledged(bool _appDirWarningAcknowledged);

signals:
    void cardDatabasePathChanged();
    void picsPathChanged();
    void themeChanged();

public:
    explicit PathsSettings(const QString &settingPath, QObject *parent = nullptr);

private:
    PathsSettings(const PathsSettings & /*other*/);
};

#endif // PATHS_SETTINGS_H
