#include "mana_base_widget.h"

#include "../../../../deck/deck_loader.h"
#include "../../../../game/cards/card_database.h"
#include "../../../../game/cards/card_database_manager.h"
#include "../../../../main.h"
#include "../general/display/bar_widget.h"

#include <QRegularExpression>
#include <decklist.h>
#include <regex>
#include <unordered_map>

ManaBaseWidget::ManaBaseWidget(QWidget *parent, DeckListModel *deck_list_model)
    : QWidget(parent), deck_list_model(deck_list_model)
{
    layout = new QHBoxLayout(this);
    this->setLayout(layout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(0, 0);
}

std::unordered_map<char, int> ManaBaseWidget::analyzeManaBase()
{
    manaBaseMap = std::unordered_map<char, int>();
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
                    auto devotion = determineManaProduction(info->getText());
                    mergeManaCounts(manaBaseMap, devotion);
                }
            }
        }
    }

    update();
    // this->setCurve(ManaBaseMap);
    // mana_curve_widget->updateDisplay();
    this->updateDisplay();
    update();
    return manaBaseMap;
}

void ManaBaseWidget::updateDisplay()
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
    for (auto entry : manaBaseMap) {
        totalSum += entry.second;
    }

    // Add new widgets to the layout
    for (auto entry : manaBaseMap) {
        BarWidget *barWidget = new BarWidget(QString(entry.first), entry.second, totalSum, this);
        layout->addWidget(barWidget);
    }

    update(); // Update the widget display
}

std::unordered_map<char, int> ManaBaseWidget::determineManaProduction(const QString &rulesText)
{
    // Initialize mana counts
    std::unordered_map<char, int> manaCounts = {{'W', 0}, {'U', 0}, {'B', 0}, {'R', 0}, {'G', 0}, {'C', 0}};

    // Define regex patterns for different mana production rules
    QRegularExpression tapAddColorless(R"(\{T\}:\s*Add\s*\{C\}|Add\s*one\s*colorless\s*mana)");
    QRegularExpression tapAddAnyColor(
        R"(\{T\}:\s*Add\s*one\s*mana\s*of\s*any\s*color|Add\s*one\s*mana\s*of\s*any\s*color)");
    QRegularExpression tapAddSpecificColor(
        R"(\{T\}:\s*Add\s*\{(W|U|B|R|G)\}|Add\s*one\s*(white|blue|black|red|green)\s*mana)");

    // Check for mana production rules in the text
    if (tapAddColorless.match(rulesText).hasMatch()) {
        manaCounts['C'] += 1; // Adds colorless mana
    }

    if (tapAddAnyColor.match(rulesText).hasMatch()) {
        manaCounts['W'] += 1; // Assumes at least 1 of any color can be produced
        manaCounts['U'] += 1;
        manaCounts['B'] += 1;
        manaCounts['R'] += 1;
        manaCounts['G'] += 1;
    }

    // Check for specific color production
    QRegularExpression specificColorRegex(R"(\{T\}:\s*Add\s*\{(W|U|B|R|G)\})");
    auto match = specificColorRegex.match(rulesText);
    if (match.hasMatch()) {
        QString color = match.captured(1);
        if (color == "W")
            manaCounts['W'] += 1;
        else if (color == "U")
            manaCounts['U'] += 1;
        else if (color == "B")
            manaCounts['B'] += 1;
        else if (color == "R")
            manaCounts['R'] += 1;
        else if (color == "G")
            manaCounts['G'] += 1;
    }

    return manaCounts;
}

void ManaBaseWidget::mergeManaCounts(std::unordered_map<char, int> &manaCounts1,
                                     const std::unordered_map<char, int> &manaCounts2)
{
    for (const auto &pair : manaCounts2) {
        manaCounts1[pair.first] += pair.second; // Add values for matching keys
    }
}
