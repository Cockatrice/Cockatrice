#include "draw_probability_calculator.h"

#include <QRegularExpression>
#include <QStringList>
#include <algorithm>
#include <cmath>
#include <libcockatrice/card/database/card_database_querier.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>

namespace
{
QStringList subtypes(const QString &typeLine)
{
    // The delimiter is spaced: a hyphen inside a subtype such as Assembly-Worker is part of its name.
    static const QRegularExpression delimiter(QStringLiteral("\\s+[-—]\\s+"));
    static const QRegularExpression whitespace(QStringLiteral("\\s+"));
    QStringList result;
    for (const auto &face : typeLine.split(QStringLiteral("//"))) {
        const auto match = delimiter.match(face);
        if (match.hasMatch()) {
            result.append(face.mid(match.capturedEnd()).split(whitespace, Qt::SkipEmptyParts));
        }
    }
    result.removeDuplicates();
    return result;
}

long double logCombination(qint64 population, int selected)
{
    const qint64 factors = std::min<qint64>(selected, population - selected);
    long double value = 0;
    for (qint64 i = 1; i <= factors; ++i) {
        value += std::log(static_cast<long double>(population - factors + i) / i);
    }
    return value;
}
} // namespace

DrawProbabilityPopulation
collectDrawProbabilityPopulation(const DeckList &deck, const CardDatabaseQuerier &query, const QString &criteria)
{
    DrawProbabilityPopulation result;
    for (const auto *node : deck.getCardNodes({DECK_ZONE_MAIN})) {
        const int count = node->getNumber();
        if (count <= 0) {
            continue;
        }
        result.totalCards += count;
        const auto info = query.getCardInfo(node->getName());
        QStringList categories;
        if (criteria == QStringLiteral("name")) {
            categories.append(info ? info->getName() : node->getName());
        } else if (!info) {
            result.unclassifiedCards += count;
            continue;
        } else if (criteria == QStringLiteral("type")) {
            categories = info->getMainCardType().split(' ', Qt::SkipEmptyParts);
            if (categories.isEmpty()) {
                result.unclassifiedCards += count;
            }
        } else if (criteria == QStringLiteral("subtype")) {
            categories = subtypes(info->getCardType());
            if (info->getCardType().trimmed().isEmpty()) {
                result.unclassifiedCards += count;
            }
        } else if (criteria == QStringLiteral("cmc")) {
            bool valid = false;
            const double manaValue = info->getCmc().toDouble(&valid);
            if (valid && std::isfinite(manaValue) && manaValue >= 0) {
                categories.append(QString::number(manaValue));
            } else {
                result.unclassifiedCards += count;
            }
        }
        categories.removeDuplicates();
        for (const auto &category : categories) {
            result.categoryCounts[category] += count;
        }
    }
    return result;
}

std::optional<double> calculateDrawProbability(qint64 totalCards, qint64 copies, int drawn, int quantity, bool atLeast)
{
    if (totalCards < 0 || copies < 0 || copies > totalCards || drawn < 0 || drawn > totalCards || quantity < 0) {
        return std::nullopt;
    }
    const int minimum = static_cast<int>(std::max<qint64>(0, drawn - (totalCards - copies)));
    const int maximum = static_cast<int>(std::min<qint64>(drawn, copies));
    if (quantity > maximum || (!atLeast && quantity < minimum)) {
        return 0.0;
    }
    if (atLeast && quantity <= minimum) {
        return 1.0;
    }

    const long double denominator = logCombination(totalCards, drawn);
    long double probability = 0;
    const int last = atLeast ? maximum : quantity;
    for (int k = quantity; k <= last; ++k) {
        probability +=
            std::exp(logCombination(copies, k) + logCombination(totalCards - copies, drawn - k) - denominator);
    }
    return static_cast<double>(std::clamp(probability, 0.0L, 1.0L));
}
