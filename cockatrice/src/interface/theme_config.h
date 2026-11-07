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
    /// Point-size range the theme font editor can represent. CardText is
    /// resolved through the same bounds where it is rendered, so the board never
    /// draws a size the editor cannot show and would clamp away on save.
    static constexpr int MIN_FONT_SIZE = 6;
    static constexpr int MAX_FONT_SIZE = 72;
    /// CardText shrink-to-fit floor, editable below the general font floor.
    static constexpr int CARD_TEXT_MIN_SIZE = 1;
    /// CardText size when neither CardTextMaxSize nor a CardText family is set.
    static constexpr int DEFAULT_CARD_TEXT_SIZE = 12;

    QMap<AppFont::Role, QFont> fonts;
    int cardTextMinSize = CARD_TEXT_MIN_SIZE;
    /// Optional size-only maximum for CardText, used by the retired maxFontSize
    /// migration so seeding it does not have to pin a family.
    int cardTextMaxSize = 0;

    /// CardText min/max resolved to what the font editor shows and a save would
    /// write: the stored values (max falling back to the CardText point size,
    /// then the default) clamped into the representable range. A seeded legacy
    /// size above the editor's maximum therefore renders at that maximum instead
    /// of at a size the editor would silently rewrite.
    int resolvedCardTextMinSize() const;
    int resolvedCardTextMaxSize() const;

    bool operator==(const FontConfig &rhs) const
    {
        return fonts == rhs.fonts && cardTextMinSize == rhs.cardTextMinSize && cardTextMaxSize == rhs.cardTextMaxSize;
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
