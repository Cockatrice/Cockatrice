#ifndef CONTINUUMATRICE_ESSENCE_CHART_COLORS_H
#define CONTINUUMATRICE_ESSENCE_CHART_COLORS_H

#include <QColor>
#include <QList>
#include <QMap>
#include <QPair>
#include <QSet>
#include <QString>
#include <libcockatrice/card/essence_types.h>

// CHART-ONLY presentation palette, NOT canon Essence colors or affiliations.
// Each type must remain distinguishable without inventing lore. N is colorless.
namespace EssenceCharts
{
inline QColor colorFor(const QString &code)
{
    if (code == QStringLiteral("N")) {
        return QColor(145, 149, 156);
    }
    const int index = Essence::symbols().indexOf(code);
    if (index < 0) {
        return QColor(145, 149, 156);
    }
    return QColor::fromHsv((index * 137) % 360, 125, 210);
}

inline QList<QPair<QString, int>> ordered(const QMap<QString, int> &values)
{
    QList<QPair<QString, int>> result;
    QSet<QString> used;
    for (const QChar code : Essence::symbols()) {
        const QString key(code);
        auto it = values.constFind(key);
        if (it != values.cend()) {
            result.append(qMakePair(it.key(), it.value()));
            used.insert(key);
        }
    }
    for (auto it = values.cbegin(); it != values.cend(); ++it) {
        if (!used.contains(it.key())) {
            result.append(qMakePair(it.key(), it.value()));
        }
    }
    return result;
}
} // namespace EssenceCharts

#endif // CONTINUUMATRICE_ESSENCE_CHART_COLORS_H
