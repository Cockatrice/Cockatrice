#ifndef COCKATRICE_DECK_LIST_STATISTICS_ANALYZER_H
#define COCKATRICE_DECK_LIST_STATISTICS_ANALYZER_H

#include "deck_list_model.h"

#include <QHash>
#include <QObject>
#include <unordered_map>

class DeckListModel;

struct DeckListStatisticsAnalyzerConfig
{
    bool computeManaBase = true;
    bool computeManaCurve = true;
    bool computeDevotion = true;
};

class DeckListStatisticsAnalyzer : public QObject
{
    Q_OBJECT

public:
    explicit DeckListStatisticsAnalyzer(QObject *parent,
                                        DeckListModel *model,
                                        DeckListStatisticsAnalyzerConfig cfg = DeckListStatisticsAnalyzerConfig());

    void update();

    [[nodiscard]] const QHash<QString, int> &getManaBase() const
    {
        return manaBaseMap;
    }
    [[nodiscard]] const std::unordered_map<int, int> &getManaCurve() const
    {
        return manaCurveMap;
    }
    [[nodiscard]] const std::unordered_map<char, int> &getDevotion() const
    {
        return manaDevotionMap;
    }

signals:
    void statsUpdated();

private:
    DeckListModel *model;
    DeckListStatisticsAnalyzerConfig config;

    // Internal result containers
    QHash<QString, int> manaBaseMap;
    std::unordered_map<int, int> manaCurveMap;
    std::unordered_map<char, int> manaDevotionMap;

    // Internal helper functions
    QHash<QString, int> determineManaProduction(const QString &);
    std::unordered_map<char, int> countManaSymbols(const QString &);
};

#endif // COCKATRICE_DECK_LIST_STATISTICS_ANALYZER_H
