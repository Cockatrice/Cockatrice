#ifndef COCKATRICE_PALETTE_GENERATOR_H
#define COCKATRICE_PALETTE_GENERATOR_H

#include "../theme_config.h"

#include <QColor>
#include <QString>

// All QPalette roles are derived from a single accent color and an
// intensity value (0-100).  See the .cpp for the full band breakdown.
namespace PaletteGenerator
{
PaletteConfig fromAccent(const QColor &accent, int intensity, const QString &scheme);
} // namespace PaletteGenerator

#endif // COCKATRICE_PALETTE_GENERATOR_H