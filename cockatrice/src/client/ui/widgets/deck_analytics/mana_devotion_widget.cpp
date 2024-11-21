#include "mana_devotion_widget.h"

#include "../../../../deck/deck_loader.h"
#include "../../../../game/cards/card_database.h"
#include "../../../../game/cards/card_database_manager.h"
#include "../../../../main.h"
#include "../general/display/bar_widget.h"

#include <decklist.h>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>

ManaDevotionWidget::ManaDevotionWidget(QWidget *parent, DeckListModel *deck_list_model)
    : QWidget(parent), deck_list_model(deck_list_model)
{
    layout = new QHBoxLayout(this);
    this->setLayout(layout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(0, 0);
}

std::unordered_map<char, int> ManaDevotionWidget::analyzeManaDevotion()
{
    manaDevotionMap = std::unordered_map<char, int>();
    InnerDecklistNode *listRoot = deck_list_model->getDeckList()->getRoot();
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

    update();
    // this->setCurve(manaDevotionMap);
    // mana_curve_widget->updateDisplay();
    this->updateDisplay();
    update();
    return manaDevotionMap;
}

void ManaDevotionWidget::updateDisplay()
{

    // Clear the layout first
    if (layout != nullptr) {
        QLayoutItem *item;
        while ((item = layout->takeAt(0)) != nullptr) {
            delete item->widget(); // Delete the widget
            delete item;           // Delete the layout item
        }
    }

    // If layout is null, create a new layout, otherwise reuse the existing one
    if (layout == nullptr) {
        layout = new QHBoxLayout(this);
        this->setLayout(layout);
    }

    int totalSum = 0;
    for (auto entry : manaDevotionMap) {
        totalSum += entry.second;
    }

    // Add new widgets to the layout
    for (auto entry : manaDevotionMap) {
        BarWidget *barWidget = new BarWidget(QString(entry.first), entry.second, totalSum, this);
        layout->addWidget(barWidget);
    }

    update(); // Update the widget display
}

// Function to count mana symbols W, U, B, R, G in the input string
std::unordered_map<char, int> ManaDevotionWidget::countManaSymbols(const QString &manaString)
{
    // Only track counts for W, U, B, R, G
    std::unordered_map<char, int> manaCounts = {{'W', 0}, {'U', 0}, {'B', 0}, {'R', 0}, {'G', 0}};

    // Convert QString to std::string for regex processing
    std::string stdManaString = manaString.toStdString();
    std::regex regexPattern(R"(\{([WUBRG])\/([WUBRG])\})");
    std::string::const_iterator searchStart(stdManaString.cbegin());
    std::smatch match;

    // Go through the string and find pairs within {}/ for W, U, B, R, G
    while (std::regex_search(searchStart, stdManaString.cend(), match, regexPattern)) {
        char mana1 = match[1].str()[0];
        char mana2 = match[2].str()[0];

        // Increment the count for both mana symbols
        manaCounts[mana1]++;
        manaCounts[mana2]++;

        // Move past the current match
        searchStart = match.suffix().first;
    }

    // Now go through the string again but ignore the {}/ parts and non-WUBRG symbols
    for (int i = 0; i < manaString.size(); ++i) {
        if (manaString[i] == '{') {
            // Skip over the {X/Y} pattern
            while (manaString[i] != '}' && i < manaString.size()) {
                ++i;
            }
        } else if (manaCounts.find(manaString[i].toLatin1()) != manaCounts.end()) {
            // If it's one of W, U, B, R, G, count it
            manaCounts[manaString[i].toLatin1()]++;
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
