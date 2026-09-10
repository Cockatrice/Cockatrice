#ifndef COCKATRICE_THEME_CONFIG_H
#define COCKATRICE_THEME_CONFIG_H

#include <QColor>
#include <QMap>
#include <QObject>
#include <QPalette>
#include <QString>

// Application-specific color roles, layered on top of the fixed QPalette role
// set. Stored in the same palette-<scheme>.toml under an [AppColors] section
// and editable from the palette editor, so theme authors can control colors
// beyond what Qt's palette can express.
namespace AppColor
{
Q_NAMESPACE
enum Role
{
    AccentStrong,
    AccentSoft,
};
Q_ENUM_NS(Role)
} // namespace AppColor

struct ThemeConfig
{
    QString colorScheme;
    QString styleName;

    bool isEmpty() const;
    QString toIni() const;

    static ThemeConfig fromThemeDir(const QString &themeDirPath);
    bool save(const QString &themeDirPath) const;
};

struct PaletteConfig
{
    QMap<QPalette::ColorGroup, QMap<QPalette::ColorRole, QColor>> colors;
    QMap<AppColor::Role, QColor> appColors;

    bool hasPalette() const;
    QString toToml() const;

    static QString fileName(const QString &colorScheme);

    static PaletteConfig fromFile(const QString &filePath);
    static PaletteConfig fromScheme(const QString &themeDirPath, const QString &colorScheme);
    static PaletteConfig fromDefault(const QString &themeDirPath, const QString &colorScheme);

    QPalette apply(QPalette base) const;
};

#endif // COCKATRICE_THEME_CONFIG_H