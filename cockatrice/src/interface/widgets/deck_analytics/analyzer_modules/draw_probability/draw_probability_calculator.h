#ifndef COCKATRICE_DRAW_PROBABILITY_CALCULATOR_H
#define COCKATRICE_DRAW_PROBABILITY_CALCULATOR_H

#include <QMap>
#include <QString>
#include <QtGlobal>
#include <optional>

class CardDatabaseQuerier;
class DeckList;

struct DrawProbabilityPopulation
{
    qint64 totalCards = 0;
    qint64 unclassifiedCards = 0;
    QMap<QString, qint64> categoryCounts;
};

// Only cards in the main deck can be drawn. Missing metadata must not shrink the population.
DrawProbabilityPopulation
collectDrawProbabilityPopulation(const DeckList &deck, const CardDatabaseQuerier &query, const QString &criteria);

// An invalid experiment (for example, drawing more cards than the deck contains) has no odds.
// A valid experiment with an impossible number of successes has probability zero.
std::optional<double> calculateDrawProbability(qint64 totalCards, qint64 copies, int drawn, int quantity, bool atLeast);

#endif
