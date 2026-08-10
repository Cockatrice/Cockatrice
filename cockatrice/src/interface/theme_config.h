#ifndef COCKATRICE_THEME_CONFIG_H
#define COCKATRICE_THEME_CONFIG_H

#include <QColor>
#include <QMap>
#include <QPalette>
#include <QString>

struct ThemeConfig
{
    QString colorScheme;
    QString styleName;

    // Optional per-theme font families (see design-game.md). Empty = use the
    // application default / the classic "Serif" hint for card titles.
    QString displayFont;
    QString bodyFont;
    QString cardTitleFont;
    QString monoFont;

    bool isEmpty() const;
    QString toIni() const;

    static ThemeConfig fromThemeDir(const QString &themeDirPath);
    bool save(const QString &themeDirPath) const;
};

struct PaletteConfig
{
    QMap<QPalette::ColorGroup, QMap<QPalette::ColorRole, QColor>> colors;

    bool hasPalette() const;
    QString toToml() const;

    static QString fileName(const QString &colorScheme);

    static PaletteConfig fromFile(const QString &filePath);
    static PaletteConfig fromScheme(const QString &themeDirPath, const QString &colorScheme);
    static PaletteConfig fromDefault(const QString &themeDirPath, const QString &colorScheme);

    QPalette apply(QPalette base) const;
};

#endif // COCKATRICE_THEME_CONFIG_H