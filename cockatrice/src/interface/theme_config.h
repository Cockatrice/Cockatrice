#ifndef COCKATRICE_THEME_CONFIG_H
#define COCKATRICE_THEME_CONFIG_H

#include <QColor>
#include <QFont>
#include <QMap>
#include <QPalette>
#include <QSharedPointer>
#include <QString>
#include <qtmetamacros.h>

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

// Application-specific font roles, one configurable family + point size per
// purpose. Stored in a shared fonts.toml under a [Fonts] section (one file per
// theme, not split by light/dark). Only family and point size are stored;
// weight/italic are stripped on load and stay caller-side concerns.
namespace AppFont
{
Q_NAMESPACE
enum Role
{
    InterfaceDefault,
    ChatLog,
    CardText,
    DeckList,
};
Q_ENUM_NS(Role)
} // namespace AppFont

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

    bool operator==(const PaletteConfig &rhs) const
    {
        return colors == rhs.colors && appColors == rhs.appColors;
    }
    bool operator!=(const PaletteConfig &rhs) const
    {
        return !(*this == rhs);
    }
    bool hasPalette() const;
    QString toToml() const;

    static QString fileName(const QString &colorScheme);

    static PaletteConfig fromFile(const QString &filePath);
    static PaletteConfig fromScheme(const QString &themeDirPath, const QString &colorScheme);
    static PaletteConfig fromDefault(const QString &themeDirPath, const QString &colorScheme);

    QPalette apply(QPalette base) const;
};

/** @brief Per-purpose font configuration, stored as family + point size only.
 *
 * Serialized to a shared fonts.toml file with a [Fonts] section, e.g.
 * @code
 * [Fonts]
 * InterfaceDefault = "Inter, 10"
 * CardText = "Noto Serif, 12"
 * CardTextMinSize = 1
 * @endcode
 * CardText stores family + MAX size; cardTextMinSize stores the overlay
 * shrink-to-fit floor (omitted from TOML when 1).
 */
struct FontConfig
{
    QMap<AppFont::Role, QFont> fonts;
    int cardTextMinSize = 1;

    bool operator==(const FontConfig &rhs) const
    {
        return fonts == rhs.fonts && cardTextMinSize == rhs.cardTextMinSize;
    }
    bool operator!=(const FontConfig &rhs) const
    {
        return !(*this == rhs);
    }
    bool hasFonts() const;
    QString toToml() const;

    static QString fileName();

    static FontConfig fromFile(const QString &filePath);
    static FontConfig fromThemeDir(const QString &themeDirPath);
};

#endif // COCKATRICE_THEME_CONFIG_H
