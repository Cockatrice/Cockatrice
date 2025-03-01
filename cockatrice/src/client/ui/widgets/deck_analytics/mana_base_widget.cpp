#include "mana_base_widget.h"

#include "../../../../deck/deck_loader.h"
#include "../../../../game/cards/card_database.h"
#include "../../../../game/cards/card_database_manager.h"
#include "../../../../main.h"
#include "../general/display/banner_widget.h"
#include "../general/display/bar_widget.h"

#include <QHash>
#include <QRegularExpression>
#include <decklist.h>

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
        totalSum += entry;
    }

    // Define color mapping for mana types
    QHash<QString, QColor> manaColors;
    manaColors.insert("W", QColor(248, 231, 185));
    manaColors.insert("U", QColor(14, 104, 171));
    manaColors.insert("B", QColor(21, 11, 0));
    manaColors.insert("R", QColor(211, 32, 42));
    manaColors.insert("G", QColor(0, 115, 62));
    manaColors.insert("C", QColor(150, 150, 150));

    for (auto manaColor : manaBaseMap.keys()) {
        QColor barColor = manaColors.contains(manaColor) ? manaColors[manaColor] : Qt::gray;
        BarWidget *barWidget = new BarWidget(QString(manaColor), manaBaseMap[manaColor], totalSum, barColor, this);
        barLayout->addWidget(barWidget);
    }

    update();
}

QHash<QString, int> ManaBaseWidget::analyzeManaBase()
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

QHash<QString, int> ManaBaseWidget::determineManaProduction(const QString &rulesText)
{
    // Initialize mana counts
    QHash<QString, int> manaCounts;
    manaCounts.insert("W", 0);
    manaCounts.insert("U", 0);
    manaCounts.insert("B", 0);
    manaCounts.insert("R", 0);
    manaCounts.insert("G", 0);
    manaCounts.insert("C", 0);

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

void ManaBaseWidget::mergeManaCounts(QHash<QString, int> &manaCounts1, const QHash<QString, int> &manaCounts2)
{
    for (auto it = manaCounts2.constBegin(); it != manaCounts2.constEnd(); ++it) {
        manaCounts1[it.key()] += it.value();
    }
}
