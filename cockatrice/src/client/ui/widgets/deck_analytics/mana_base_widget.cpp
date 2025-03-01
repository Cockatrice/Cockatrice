#include "mana_base_widget.h"

#include "../../../../deck/deck_loader.h"
#include "../../../../game/cards/card_database.h"
#include "../../../../game/cards/card_database_manager.h"
#include "../../../../main.h"
#include "../general/display/banner_widget.h"
#include "../general/display/bar_widget.h"

#include <QRegularExpression>
#include <decklist.h>
#include <regex>
#include <unordered_map>

ManaBaseWidget::ManaBaseWidget(QWidget *parent, DeckListModel *_deckListModel)
    : QWidget(parent), deckListModel(_deckListModel)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    bannerWidget = new BannerWidget(this, tr("Mana Base"), Qt::Vertical, 100);
    bannerWidget->setMaximumHeight(100);
    layout->addWidget(bannerWidget);

    barLayout = new QHBoxLayout(this);
    layout->addLayout(barLayout);

    connect(deckListModel, &DeckListModel::dataChanged, this, &ManaBaseWidget::analyzeManaBase);

    retranslateUi();
}

void ManaBaseWidget::retranslateUi()
{
    bannerWidget->setText(tr("Mana Base"));
}

void ManaBaseWidget::setDeckModel(DeckListModel *deckModel)
{
    deckListModel = deckModel;
    connect(deckListModel, &DeckListModel::dataChanged, this, &ManaBaseWidget::analyzeManaBase);
    analyzeManaBase();
}

void ManaBaseWidget::updateDisplay()
{
    // Clear the layout first
    QLayoutItem *item;
    while ((item = barLayout->takeAt(0)) != nullptr) {
        item->widget()->deleteLater();
        delete item;
    }

    int totalSum = 0;
    for (auto entry : manaBaseMap) {
        totalSum += entry.second;
    }

    // Define color mapping for mana types
    std::unordered_map<QString, QColor> manaColors = {{"W", QColor(248, 231, 185)}, {"U", QColor(14, 104, 171)},
                                                      {"B", QColor(21, 11, 0)},     {"R", QColor(211, 32, 42)},
                                                      {"G", QColor(0, 115, 62)},    {"C", QColor(150, 150, 150)}};

    for (auto entry : manaBaseMap) {
        QColor barColor = manaColors.count(entry.first) ? manaColors[entry.first] : Qt::gray;
        BarWidget *barWidget = new BarWidget(QString(entry.first), entry.second, totalSum, barColor, this);
        barLayout->addWidget(barWidget);
    }

    update();
}

std::unordered_map<QString, int> ManaBaseWidget::analyzeManaBase()
{
    manaBaseMap.clear();
    InnerDecklistNode *listRoot = deckListModel->getDeckList()->getRoot();
    for (int i = 0; i < listRoot->size(); i++) {
        InnerDecklistNode *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
        for (int j = 0; j < currentZone->size(); j++) {
            DecklistCardNode *currentCard = dynamic_cast<DecklistCardNode *>(currentZone->at(j));
            if (!currentCard)
                continue;

            for (int k = 0; k < currentCard->getNumber(); ++k) {
                CardInfoPtr info = CardDatabaseManager::getInstance()->getCard(currentCard->getName());
                if (info) {
                    auto devotion = determineManaProduction(info->getText());
                    mergeManaCounts(manaBaseMap, devotion);
                }
            }
        }
    }

    updateDisplay();
    return manaBaseMap;
}

std::unordered_map<QString, int> ManaBaseWidget::determineManaProduction(const QString &rulesText)
{
    // Initialize mana counts
    std::unordered_map<QString, int> manaCounts = {{"W", 0}, {"U", 0}, {"B", 0}, {"R", 0}, {"G", 0}, {"C", 0}};

    // Define regex patterns for different mana production rules
    QRegularExpression tapAddColorless(R"(\{T\}:\s*Add\s*\{C\}|Add\s*one\s*colorless\s*mana)");
    QRegularExpression tapAddAnyColor(
        R"(\{T\}:\s*Add\s*one\s*mana\s*of\s*any\s*color|Add\s*one\s*mana\s*of\s*any\s*color)");
    QRegularExpression tapAddSpecificColor(
        R"(\{T\}:\s*Add\s*\{(W|U|B|R|G)\}|Add\s*one\s*(white|blue|black|red|green)\s*mana)");

    // Check for mana production rules in the text
    if (tapAddColorless.match(rulesText).hasMatch()) {
        manaCounts["C"] += 1; // Adds colorless mana
    }

    if (tapAddAnyColor.match(rulesText).hasMatch()) {
        manaCounts["W"] += 1; // Assumes at least 1 of any color can be produced
        manaCounts["U"] += 1;
        manaCounts["B"] += 1;
        manaCounts["R"] += 1;
        manaCounts["G"] += 1;
    }

    // Check for specific color production
    QRegularExpression specificColorRegex(R"(\{T\}:\s*Add\s*\{(W|U|B|R|G)\})");
    auto match = specificColorRegex.match(rulesText);
    if (match.hasMatch()) {
        QString color = match.captured(1);
        manaCounts[color] += 1;
    }

    return manaCounts;
}

void ManaBaseWidget::mergeManaCounts(std::unordered_map<QString, int> &manaCounts1,
                                     const std::unordered_map<QString, int> &manaCounts2)
{
    for (const auto &pair : manaCounts2) {
        manaCounts1[pair.first] += pair.second; // Add values for matching keys
    }
}

