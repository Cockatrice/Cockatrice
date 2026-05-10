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

    bool isEmpty() const;
    QString toIni() const;
};

struct PaletteConfig
{
    QMap<QPalette::ColorGroup, QMap<QPalette::ColorRole, QColor>> colors;

    bool hasPalette() const;
    QString toToml() const;
};

namespace ThemeConfigParser
{

ThemeConfig parseThemeConfig(const QString &themeDirPath);

bool saveThemeConfig(const QString &themeDirPath, const ThemeConfig &cfg);

QString paletteFileName(const QString &colorScheme);

PaletteConfig parsePalette(const QString &filePath);

PaletteConfig parsePaletteForScheme(const QString &themeDirPath, const QString &colorScheme);

PaletteConfig parsePaletteDefault(const QString &themeDirPath, const QString &colorScheme);

QPalette applyToPalette(const PaletteConfig &cfg, QPalette base);

} // namespace ThemeConfigParser

#endif // COCKATRICE_THEME_CONFIG_H