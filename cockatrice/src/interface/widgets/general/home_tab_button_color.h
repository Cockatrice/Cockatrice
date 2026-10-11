#ifndef COCKATRICE_HOME_TAB_BUTTON_COLOR_H
#define COCKATRICE_HOME_TAB_BUTTON_COLOR_H

#include <QList>

namespace HomeTabButtonColor
{

/**
 * @brief Where to get the colors for the home tab buttons from
 */
enum Source
{
    FromThemeColors, ///< Use the theme's identity accent colors
    FromBackground,  ///< Extract colour from the background image
};

struct Entry
{
    Source source;
    const char *trKey; ///< key for translation
};

inline QList<Entry> all()
{
    static QList<Entry> entries = {{FromThemeColors, QT_TR_NOOP("From theme colors")},
                                   {FromBackground, QT_TR_NOOP("Extract from background")}};

    return entries;
}

/**
 * Safely converts an int into the corresponding Source.
 *
 * @param value The int value
 * @return The Source. Returns Source::FromThemeColors if the value is not within range
 */
inline Source intToSource(int value)
{
    if (value > FromBackground) {
        return FromThemeColors; // default
    }

    return static_cast<Source>(value);
}

} // namespace HomeTabButtonColor

#endif // COCKATRICE_HOME_TAB_BUTTON_COLOR_H
