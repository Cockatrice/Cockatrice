/**
 * @file theme_manager.h
 * @ingroup CoreSettings
 */
//! \todo Document this file.

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "theme_config.h"

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QLoggingCategory>
#include <QMap>
#include <QObject>
#include <QPalette>
#include <QString>
#include <QStringView>
#include <array>
#include <qtmetamacros.h>

inline Q_LOGGING_CATEGORY(ThemeManagerLog, "theme_manager");

typedef QMap<QString, QString> QStringMap;
typedef QMap<int, QBrush> QBrushMap;

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    ThemeManager(QObject *parent = nullptr);

    enum Role
    {
        MinRole = 0,
        Hand = MinRole,
        Stack,
        Table,
        Player,
        MaxRole = Player,
    };

private:
    QString defaultStyleName;
    // Pristine application palette captured at startup, before any custom theme
    // palette is applied. Used as the base when a theme supplies no palette, so
    // switching away from a custom palette restores the original colours.
    QPalette defaultPalette;
    // Pristine application font captured at startup, before any theme font is
    // applied. Restored when a theme (or a preview) supplies no InterfaceDefault
    // font, so switching away from a themed font cannot leave it behind.
    QFont defaultFont;
    QString currentThemePath;
    std::array<QBrush, Role::MaxRole + 1> brushes;
    QStringMap availableThemes;
    QMap<AppColor::Role, QColor> currentAppColors;
    FontConfig currentFonts;
    /*
      Internal cache for multiple backgrounds
    */
    std::array<QBrushMap, Role::MaxRole + 1> brushesCache;

protected:
    void ensureThemeDirectoryExists();
    QBrush loadBrush(QString fileName, QColor fallbackColor);
    QBrush loadExtraBrush(QString fileName, QBrush &fallbackBrush);
    void applyStyleAndPalette(const QString &themeName,
                              const ThemeConfig &themeCfg,
                              const PaletteConfig &palCfg,
                              const QString &activeScheme);

public:
    // Resolves the directory to write theme changes to for the given theme
    // name. The resolved theme dir (user or system) is used when writable;
    // read-only system themes fall back to the user themes directory, creating
    // it if needed, so customisations never get lost on upgrade.
    static QString writableThemeDir(const QString &themeName);
    // Carries a user profile's customised "Default" theme directory over to the
    // "System" directory the theme was renamed to, so zone graphics,
    // stylesheets, palettes and theme.cfg survive the rename. Renames the
    // directory when "System" does not exist yet, otherwise merges file by
    // file without overwriting what "System" already has. No-op unless the
    // active theme is one of the two names, so a hand-made "Default" folder of
    // a user on another theme is left alone.
    static void migrateLegacyThemeDir(const QString &themesPath, const QString &currentThemeName);
    // Probe whether a directory is truly writable by trying to create and remove
    // a temporary file. QFileInfo::isWritable() on a directory is unreliable
    // (notably on Windows where UAC VirtualStore can make a system dir appear
    // writable).
    static bool isDirReallyWritable(const QString &dirPath);
    // Explicit color scheme of the theme: theme.cfg's ColorScheme setting
    // (Dark/Light), falling back to the OS color scheme when it is "System".
    bool isDarkMode(const QString &themeDirPath) const;
    // The resolved scheme of the currently active theme.
    bool isDarkModeActive() const
    {
        return isDarkMode(currentThemePath);
    }
    QStringMap &getAvailableThemes();
    // Returns the path to the currently active theme directory (empty = default)
    QString getCurrentThemePath() const
    {
        return currentThemePath;
    }
    // Load the global theme settings (style + color scheme preference)
    static ThemeConfig loadGlobalConfig(const QString &themeDirPath);
    static bool saveGlobalConfig(const QString &themeDirPath, const ThemeConfig &cfg);

    // Load/save per-scheme palette colors
    static PaletteConfig loadPaletteConfig(const QString &themeDirPath, const QString &colorScheme);
    static bool savePaletteConfig(const QString &themeDirPath, const QString &colorScheme, const PaletteConfig &cfg);
    // Load/save shared per-theme fonts (fonts.toml, not split by scheme)
    static FontConfig loadFontConfig(const QString &themeDirPath);
    static bool saveFontConfig(const QString &themeDirPath, const FontConfig &cfg);
    // Resolve prefix to a scheme-qualified "theme:" path. Existence is probed
    // internally across the formats themes may ship (.png/.jpg/.svg), so
    // callers load the returned path directly. Prefers "<prefix>-<dark|light>"
    // when a file exists at that stem, otherwise the plain "<prefix>" as the
    // super fallback. The resolved scheme covers explicit light/dark as well
    // as OS-resolved "system". Returns the path with its file extension when a
    // match is found; unqualified assets keep working unchanged.
    QString assetPath(QStringView prefix) const;
    // Like assetPath, but resolves only the scheme-qualified variant
    // ("<prefix>-<dark|light>.<ext>") and returns an empty string when no
    // variant exists — it never falls back to the plain "<prefix>" asset.
    // Callers that must distinguish "no authored variant" (e.g. to keep a
    // legacy runtime fallback alive) should use this instead of assetPath.
    QString schemeVariantPath(QStringView prefix) const;
    // Load the theme's shipped default palette, falling back to the system
    // theme directory when it is absent from the resolved (user) directory.
    static PaletteConfig
    loadDefaultPaletteConfig(const QString &themeDirPath, const QString &themeName, const QString &colorScheme);
    // Load the theme's shipped default fonts, falling back to the system theme
    // directory when fonts.toml is absent from the resolved (user) directory.
    static FontConfig loadDefaultFontConfig(const QString &themeDirPath, const QString &themeName);
    /** @brief Writes cfg to disk as the theme's palette-<scheme>.toml and updates the
     *         theme's stored colour scheme to match. Shared by PaletteEditorDialog::onSave
     *         and FirstRunWizard's theme step so the two "generate + keep" paths can't drift. */
    static bool commitPalette(const QString &themeDirPath, const QString &colorScheme, const PaletteConfig &cfg);
    void setColorScheme(const QString &scheme);
    void setStyleName(const QString &styleName);

    void reloadCurrentTheme();
    void previewPalette(const PaletteConfig &cfg, const QString &scheme);
    void previewFonts(const FontConfig &cfg);

    // Resolves an application color role: the theme's stored [AppColors] value
    // when present, otherwise a palette-accent-derived fallback.
    QColor appColor(AppColor::Role role) const;
    /** @brief Resolves an application font role: the theme's stored [Fonts]
     *         value when present, otherwise the application font. Roles that
     *         had a historical hardcoded family ask for it through
     *         cardTextFont() and deckListFont() instead. */
    QFont appFont(AppFont::Role role) const;
    /** @brief Whether the active theme defines this role in fonts.toml. */
    bool hasAppFont(AppFont::Role role) const;
    /** @brief The CardText role with its historical Serif fallback. Used by the
     *         counter painters, which were Serif before the role existed. */
    QFont cardTextFont() const;
    /** @brief The DeckList role with its historical Serif fallback. Used by the
     *         two surfaces that hardcoded Serif before the role existed (the
     *         deck view zone headers and the printed deck list); every other
     *         DeckList reader falls back to the application font. */
    QFont deckListFont() const;
    /** @brief Resolves an application font role against the startup font only,
     *         ignoring any previewed config in currentFonts. Editors that show
     *         "what this role resolves to" must use this, because appFont()
     *         reflects a live preview while the editor is open. */
    QFont defaultAppFont(AppFont::Role role) const;
    /** @brief CardText max size: stored CardText point size, else 12. */
    int cardTextMaxSize() const;
    /** @brief CardText shrink-to-fit floor: stored CardTextMinSize, else 1. */
    int cardTextMinSize() const;

    QBrush &getBgBrush(Role zone);
    QBrush getExtraBgBrush(Role zone, int zoneId = 0);
protected slots:
    void themeChangedSlot();
signals:
    void themeChanged();
    void paletteChanged();
    void fontsChanged();
};

extern ThemeManager *themeManager;

#endif
