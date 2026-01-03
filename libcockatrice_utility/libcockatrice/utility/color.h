#ifndef COLOR_H
#define COLOR_H

#ifdef QT_GUI_LIB
#include <QColor>
#endif

#include <libcockatrice/protocol/pb/color.pb.h>

#ifdef QT_GUI_LIB
inline QColor convertColorToQColor(const color &c)
{
    return QColor(c.r(), c.g(), c.b());
}

inline color convertQColorToColor(const QColor &c)
{
    color result;
    result.set_r(c.red());
    result.set_g(c.green());
    result.set_b(c.blue());
    return result;
}

namespace GameSpecificColors
{
namespace MTG
{
inline QColor colorHelper(const QString &name)
{
    static const QMap<QString, QColor> colorMap = {
        {"W", QColor(245, 245, 220)},
        {"U", QColor(80, 140, 255)},
        {"B", QColor(60, 60, 60)},
        {"R", QColor(220, 60, 50)},
        {"G", QColor(70, 160, 70)},
        {"Creature", QColor(70, 130, 180)},
        {"Instant", QColor(138, 43, 226)},
        {"Sorcery", QColor(199, 21, 133)},
        {"Enchantment", QColor(218, 165, 32)},
        {"Artifact", QColor(169, 169, 169)},
        {"Planeswalker", QColor(210, 105, 30)},
        {"Land", QColor(110, 80, 50)},
    };

    if (colorMap.contains(name))
        return colorMap[name];

    if (name.length() == 1 && colorMap.contains(name.toUpper()))
        return colorMap[name.toUpper()];

    uint h = qHash(name);
    int r = 100 + (h % 120);
    int g = 100 + ((h >> 8) % 120);
    int b = 100 + ((h >> 16) % 120);

    return QColor(r, g, b);
}
} // namespace MTG
} // namespace GameSpecificColors
#endif

inline color makeColor(int r, int g, int b)
{
    color result;
    result.set_r(r);
    result.set_g(g);
    result.set_b(b);
    return result;
}

#endif
