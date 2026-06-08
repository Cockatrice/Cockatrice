#ifndef COCKATRICE_TALLY_MANAGER_H
#define COCKATRICE_TALLY_MANAGER_H

#include <QMap>
#include <QObject>

class CardItem;

/**
 * The result of a tally
 */
struct TallyResult
{
    QString text;  ///< The displayed text.
    int value = 0; ///< The computed value
};

/**
 * Responsible for managing all the tally counts and states
 */
class TallyManager : public QObject
{
    Q_OBJECT

public:
    enum EntryType
    {
        COUNT,
        TOTAL_POWER,
        TOTAL_MANA_VALUE
    };

    static const QList<EntryType> ENTRY_TYPES;

    explicit TallyManager(QObject *parent = nullptr);

    const QMap<EntryType, TallyResult> &getAllResults() const;

public slots:
    void updateSelection(const QList<CardItem *> &cards);

signals:
    void tallyChanged(const QMap<EntryType, TallyResult> &results);

private:
    QMap<EntryType, TallyResult> resultsMap;

    static TallyResult computeTallyResult(EntryType type, const QList<CardItem *> &cards);

    void updateResultsMap(const QList<CardItem *> &cards);
};

#endif // COCKATRICE_TALLY_MANAGER_H
