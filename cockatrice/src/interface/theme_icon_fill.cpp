#include "theme_icon_fill.h"

namespace
{
// The bottom end of every filled treatment. The authored tone is the top, so a
// single darkening keeps the emphasis level the palette author picked and still
// gives the shape the lit-from-above depth the pre-#7595 art had.
constexpr int GRADIENT_BOTTOM_DARKER = 140;
} // namespace

namespace ThemeIconFill
{
QColor topColor(const QColor &tone)
{
    return tone;
}

QColor bottomColor(const QColor &tone)
{
    return tone.darker(GRADIENT_BOTTOM_DARKER);
}
} // namespace ThemeIconFill
