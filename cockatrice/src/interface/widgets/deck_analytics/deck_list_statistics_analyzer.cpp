#include "deck_list_statistics_analyzer.h"

#include "deck_list_model.h"
#include "deck_list_statistics_analyzer.h"

#include <QRegularExpression>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/deck_list.h>

DeckListStatisticsAnalyzer::DeckListStatisticsAnalyzer(QObject *parent,
                                                       DeckListModel *_model,
                                                       DeckListStatisticsAnalyzerConfig cfg)
    : QObject(parent), model(_model), config(cfg)
{
    connect(model, &DeckListModel::dataChanged, this, &DeckListStatisticsAnalyzer::update);
}

void DeckListStatisticsAnalyzer::update()
{
    manaBaseMap.clear();
    manaCurveMap.clear();
    manaDevotionMap.clear();

    auto nodes = model->getDeckList()->getCardNodes();

    for (auto *node : nodes) {
        CardInfoPtr info = CardDatabaseManager::query()->getCardInfo(node->getName());
        if (!info)
            continue;

        for (int i = 0; i < node->getNumber(); ++i) {
            // ---- Mana curve ----
            if (config.computeManaCurve) {
                manaCurveMap[info->getCmc().toInt()]++;
            }

            // ---- Mana base ----
            if (config.computeManaBase) {
                auto mana = determineManaProduction(info->getText());
                for (auto it = mana.begin(); it != mana.end(); ++it)
                    manaBaseMap[it.key()] += it.value();
            }

            // ---- Devotion ----
            if (config.computeDevotion) {
                auto devo = countManaSymbols(info->getManaCost());
                for (auto &d : devo)
                    manaDevotionMap[d.first] += d.second;
            }
        }
    }

    emit statsUpdated();
}

QHash<QString, int> DeckListStatisticsAnalyzer::determineManaProduction(const QString &rulesText)
{
    QHash<QString, int> manaCounts = {{"W", 0}, {"U", 0}, {"B", 0}, {"R", 0}, {"G", 0}, {"C", 0}};

    QString text = rulesText.toLower(); // Normalize case for matching

    // Quick keyword-based checks for any color and colorless mana
    if (text.contains("{t}: add one mana of any color") || text.contains("add one mana of any color")) {
        for (const auto &color : {QStringLiteral("W"), QStringLiteral("U"), QStringLiteral("B"), QStringLiteral("R"),
                                  QStringLiteral("G")}) {
            manaCounts[color]++;
        }
    }
    if (text.contains("{t}: add {c}") || text.contains("add one colorless mana")) {
        manaCounts["C"]++;
    }

    // Optimized regex for specific mana symbols
    static const QRegularExpression specificColorRegex(R"(\{T\}:\s*Add\s*\{([WUBRG])\})");
    QRegularExpressionMatch match = specificColorRegex.match(rulesText);
    if (match.hasMatch()) {
        manaCounts[match.captured(1)]++;
    }

    return manaCounts;
}

std::unordered_map<char, int> DeckListStatisticsAnalyzer::countManaSymbols(const QString &manaString)
{
    std::unordered_map<char, int> manaCounts = {{'W', 0}, {'U', 0}, {'B', 0}, {'R', 0}, {'G', 0}};

    int len = manaString.length();
    for (int i = 0; i < len; ++i) {
        if (manaString[i] == '{') {
            ++i; // Move past '{'
            if (i < len && manaCounts.find(manaString[i].toLatin1()) != manaCounts.end()) {
                char mana1 = manaString[i].toLatin1();
                ++i; // Move to next character
                if (i < len && manaString[i] == '/') {
                    ++i; // Move past '/'
                    if (i < len && manaCounts.find(manaString[i].toLatin1()) != manaCounts.end()) {
                        char mana2 = manaString[i].toLatin1();
                        manaCounts[mana1]++;
                        manaCounts[mana2]++;
                    } else {
                        // Handle cases like "{W/}" where second part is invalid
                        manaCounts[mana1]++;
                    }
                } else {
                    manaCounts[mana1]++;
                }
            }
            // Ensure we always skip to the closing '}'
            while (i < len && manaString[i] != '}') {
                ++i;
            }
        }
        // Check if the character is a standalone mana symbol (not inside {})
        else if (manaCounts.find(manaString[i].toLatin1()) != manaCounts.end()) {
            manaCounts[manaString[i].toLatin1()]++;
        }
    }

    return manaCounts;
}
