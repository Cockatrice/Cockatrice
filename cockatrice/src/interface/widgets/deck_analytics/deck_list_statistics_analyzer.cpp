#include "deck_list_statistics_analyzer.h"

#include "deck_list_model.h"
#include "libcockatrice/card/card_info.h"
#include <libcockatrice/card/essence_types.h>
#include "libcockatrice/card/database/card_database_querier.h"
#include "libcockatrice/deck_list/tree/deck_list_card_node.h"
#include "libcockatrice/deck_list/tree/inner_deck_list_node.h"

#include <QChar>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSharedPointer>
#include <QStringLiteral>
#include <initializer_list>
#include <libcockatrice/card/database/card_database_manager.h>
#include <utility>

DeckListStatisticsAnalyzer::DeckListStatisticsAnalyzer(QObject *parent,
                                                       DeckListModel *_model,
                                                       DeckListStatisticsAnalyzerConfig _config)
    : QObject(parent), model(_model), config(_config)
{
    connect(model, &DeckListModel::cardsChanged, this, &DeckListStatisticsAnalyzer::analyze);
}

void DeckListStatisticsAnalyzer::analyze()
{
    clearData();

    QList<const DecklistCardNode *> nodes;

    if (config.includeSideboard) {
        nodes = model->getCardNodes();
    } else {
        nodes = model->getCardNodesForZone(DECK_ZONE_MAIN);
    }

    for (auto node : nodes) {
        CardInfoPtr info = CardDatabaseManager::query()->getCardInfo(node->getName());
        if (!info) {
            continue;
        }

        const int amount = node->getNumber();
        QStringList copiesOfName;
        for (int i = 0; i < amount; i++) {
            copiesOfName.append(node->getName());
        }

        // Convert once
        const int cmc = info->getCmc().toInt();
        QStringList types = info->getMainCardType().split(' ');
        QStringList subtypes = info->getCardType().split('-').last().split(" ");
        QString colors = info->getColors();
        int power = info->getPowTough().split("/").first().toInt();
        int toughness = info->getPowTough().split("/").last().toInt();

        // For each copy of card
        // ---------------- Mana Curve ----------------
        if (config.computeManaCurve) {
            manaCurveMap[cmc] += amount;
        }

        // per-type curve
        for (auto &t : types) {
            manaCurveByType[t][cmc] += amount;
            manaCurveCardsByType[t][cmc] << copiesOfName;
        }

        // Per-subtype curve
        for (auto &st : subtypes) {
            manaCurveBySubtype[st][cmc] += amount;
            manaCurveCardsBySubtype[st][cmc] << copiesOfName;
        }

        // per-color curve
        for (auto &c : colors) {
            manaCurveByColor[c][cmc] += amount;
            manaCurveCardsByColor[c][cmc] << copiesOfName;
        }

        // Power/toughness
        manaCurveByPower[QString::number(power)][cmc] += amount;
        manaCurveCardsByPower[QString::number(power)][cmc] << copiesOfName;
        manaCurveByToughness[QString::number(toughness)][cmc] += amount;
        manaCurveCardsByToughness[QString::number(toughness)][cmc] << copiesOfName;

        // ========== Category Counts ===========
        for (auto &t : types) {
            typeCount[t] += amount;
        }
        for (auto &st : subtypes) {
            subtypeCount[st] += amount;
        }
        for (auto &c : colors) {
            colorCount[c] += amount;
        }
        manaValueCount[cmc] += amount;

        // ---------------- Mana Base ----------------
        if (config.computeManaBase) {
            auto prod = determineManaProduction(info->getText());
            for (auto it = prod.begin(); it != prod.end(); ++it) {
                if (it.value() > 0) {
                    productionPipCount[it.key()] += it.value() * amount;
                    productionCardCount[it.key()] += amount;
                }
                manaBaseMap[it.key()] += it.value() * amount;
            }
        }

        // ---------------- Devotion ----------------
        if (config.computeDevotion) {
            auto devo = countManaSymbols(info->getManaCost());
            for (auto &d : devo) {
                if (d.second > 0) {
                    devotionPipCount[QString(d.first)] += d.second * amount;
                    devotionCardCount[QString(d.first)] += amount;
                }
                manaDevotionMap[d.first] += d.second * amount;
            }
        }
    }

    emit statsUpdated();
}

QHash<QString, int> DeckListStatisticsAnalyzer::determineManaProduction(const QString &rulesText)
{
    QHash<QString, int> result;
    for (const QChar code : Essence::symbols()) {
        result.insert(QString(code), 0);
    }

    // Explicit Essence grants only. Inferring additional resources from MTG
    // tap/mana rules would invent Continuum Universes gameplay semantics.
    // E.g. "Add {M}" / "add {N}" both work.
    static const QRegularExpression grantedEssence(
        QStringLiteral(R"(\badd\s*\{([MSOGEAQCJBRLVTPN])\})"),
        QRegularExpression::CaseInsensitiveOption);

    auto matches = grantedEssence.globalMatch(rulesText);
    while (matches.hasNext()) {
        const QString code = matches.next().captured(1).toUpper();
        result[code]++;
    }
    return result;
}

std::unordered_map<char, int> DeckListStatisticsAnalyzer::countManaSymbols(const QString &essenceCost)
{
    std::unordered_map<char, int> result;
    for (const QChar code : Essence::symbols()) {
        result.emplace(code.toLatin1(), 0);
    }

    for (const QString &token : Essence::parseCost(essenceCost)) {
        if (token.size() == 1 && Essence::isSymbol(token.at(0))) {
            result[token.at(0).toLatin1()]++;
        }
    }
    return result;
}

// Hypergeometric probability: P(X=k)
double DeckListStatisticsAnalyzer::hypergeometric(int N, int K, int n, int k)
{
    if (k < 0 || k > n || K > N) {
        return 0.0;
    }

    auto choose = [](int n, int r) -> double {
        if (r > n) {
            return 0.0;
        }
        if (r == 0 || r == n) {
            return 1.0;
        }
        double res = 1.0;
        for (int i = 1; i <= r; ++i) {
            res *= (n - r + i);
            res /= i;
        }
        return res;
    };

    return choose(K, k) * choose(N - K, n - k) / choose(N, n);
}

void DeckListStatisticsAnalyzer::clearData()
{
    manaBaseMap.clear();
    manaCurveMap.clear();
    manaDevotionMap.clear();

    devotionPipCount.clear();
    devotionCardCount.clear();

    productionPipCount.clear();
    productionCardCount.clear();

    manaCurveByType.clear();
    manaCurveBySubtype.clear();
    manaCurveByColor.clear();
    manaCurveByPower.clear();
    manaCurveByToughness.clear();

    manaCurveCardsByType.clear();
    manaCurveCardsBySubtype.clear();
    manaCurveCardsByColor.clear();
    manaCurveCardsByPower.clear();
    manaCurveCardsByToughness.clear();

    typeCount.clear();
    subtypeCount.clear();
    colorCount.clear();
    rarityCount.clear();
    manaValueCount.clear();
}