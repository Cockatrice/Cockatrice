/**
 * @file theme_manager.h
 * @ingroup CoreSettings
 */
//! \todo Document this file.

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "theme_config.h"

#include <QBrush>
#include <QDir>
#include <QLoggingCategory>
#include <QMap>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <array>

inline Q_LOGGING_CATEGORY(ThemeManagerLog, "theme_manager");

typedef QMap<QString, QString> QStringMap;
typedef QMap<int, QBrush> QBrushMap;

class QApplication;

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
    QString currentThemePath;
    std::array<QBrush, Role::MaxRole + 1> brushes;
    QStringMap availableThemes;
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
    bool isBuiltInTheme();
    bool isDarkMode(const QString &themeDirPath);
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
    // Load the theme's shipped default palette, falling back to the system
    // theme directory when it is absent from the resolved (user) directory.
    static PaletteConfig
    loadDefaultPaletteConfig(const QString &themeDirPath, const QString &themeName, const QString &colorScheme);
    /** @brief Writes cfg to disk as the theme's palette-<scheme>.toml and updates the
     *         theme's stored colour scheme to match. Shared by PaletteEditorDialog::onSave
     *         and FirstRunWizard's theme step so the two "generate + keep" paths can't drift. */
    static bool commitPalette(const QString &themeDirPath, const QString &colorScheme, const PaletteConfig &cfg);
    void setColorScheme(const QString &scheme);
    void setStyleName(const QString &styleName);

    void reloadCurrentTheme();
    void previewPalette(const PaletteConfig &cfg, const QString &scheme);

    QBrush &getBgBrush(Role zone);
    QBrush getExtraBgBrush(Role zone, int zoneId = 0);
protected slots:
    void themeChangedSlot();
signals:
    void themeChanged();
};

extern ThemeManager *themeManager;

#endif
