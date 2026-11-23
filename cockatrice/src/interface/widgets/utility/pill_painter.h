#ifndef PILL_PAINTER_H
#define PILL_PAINTER_H

#include <QPalette>
#include <QString>

class QPainter;
class QStyleOptionViewItem;

namespace pill_painter
{
constexpr int PAD_X = 10;
constexpr int PAD_Y = 4;

/**
 * Draws a translucent, palette-tinted rounded pill inside the option's rect and
 * paints the text elided and centred on it, so badges follow the active theme.
 * Returns false when the cell is too small to hold a pill, leaving the caller to
 * fall back to default painting.
 */
bool paint(QPainter *painter,
           const QStyleOptionViewItem &option,
           QPalette::ColorRole tintRole,
           QPalette::ColorRole textRole,
           const QString &text);

} // namespace pill_painter

#endif // PILL_PAINTER_H
