#include "mana_devotion_widget.h"

#include "../../../../deck/deck_loader.h"
#include "../../../../game/cards/card_database.h"
#include "../../../../game/cards/card_database_manager.h"
#include "../../../../main.h"
#include "../general/display/banner_widget.h"
#include "../general/display/bar_widget.h"

#include <decklist.h>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>

ManaDevotionWidget::ManaDevotionWidget(QWidget *parent, DeckListModel *_deckListModel)
    : QWidget(parent), deckListModel(_deckListModel)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    bannerWidget = new BannerWidget(this, tr("Mana Devotion"), Qt::Vertical, 100);
    bannerWidget->setMaximumHeight(100);
    layout->addWidget(bannerWidget);

    barLayout = new QHBoxLayout();
    layout->addLayout(barLayout);

    connect(deckListModel, &DeckListModel::dataChanged, this, &ManaDevotionWidget::analyzeManaDevotion);

    retranslateUi();
}

void ManaDevotionWidget::retranslateUi()
{
    bannerWidget->setText(tr("Mana Devotion"));
}

void ManaDevotionWidget::setDeckModel(DeckListModel *deckModel)
{
    deckListModel = deckModel;
    connect(deckListModel, &DeckListModel::dataChanged, this, &ManaDevotionWidget::analyzeManaDevotion);
    analyzeManaDevotion();
}

std::unordered_map<char, int> ManaDevotionWidget::analyzeManaDevotion()
{
    manaDevotionMap.clear();
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
                    auto devotion = countManaSymbols(info->getManaCost());
                    mergeManaCounts(manaDevotionMap, devotion);
                }
            }
        }
    }

    updateDisplay();
    return manaDevotionMap;
}

void ManaDevotionWidget::updateDisplay()
{
    // Clear the layout first
    QLayoutItem *item;
    while ((item = barLayout->takeAt(0)) != nullptr) {
        item->widget()->deleteLater();
        delete item;
    }

    int totalSum = 0;
    for (auto entry : manaDevotionMap) {
        totalSum += entry.second;
    }

    // Define color mapping for devotion bars
    std::unordered_map<char, QColor> manaColors = {{'W', QColor(248, 231, 185)}, {'U', QColor(14, 104, 171)},
                                                   {'B', QColor(21, 11, 0)},     {'R', QColor(211, 32, 42)},
                                                   {'G', QColor(0, 115, 62)},    {'C', QColor(150, 150, 150)}};

    for (auto entry : manaDevotionMap) {
        QColor barColor = manaColors.count(entry.first) ? manaColors[entry.first] : Qt::gray;
        BarWidget *barWidget = new BarWidget(QString(entry.first), entry.second, totalSum, barColor, this);
        barLayout->addWidget(barWidget);
    }

    update(); // Update the widget display
}

// Function to count mana symbols W, U, B, R, G in the input string
std::unordered_map<char, int> ManaDevotionWidget::countManaSymbols(const QString &manaString)
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
                    }
                } else {
                    manaCounts[mana1]++;
                }
            }
            while (i < len && manaString[i] != '}') {
                ++i; // Skip to closing '}'
            }
        }
    }

    return manaCounts;
}

void ManaDevotionWidget::mergeManaCounts(std::unordered_map<char, int> &manaCounts1,
                                         const std::unordered_map<char, int> &manaCounts2)
{
    for (const auto &pair : manaCounts2) {
        manaCounts1[pair.first] += pair.second; // Add values for matching keys
    }
}
