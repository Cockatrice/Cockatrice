#include "mana_curve_widget.h"

#include "../../../../deck/deck_loader.h"
#include "../../../../game/cards/card_database.h"
#include "../../../../game/cards/card_database_manager.h"
#include "../../../../main.h"
#include "../general/display/banner_widget.h"
#include "../general/display/bar_widget.h"

#include <decklist.h>
#include <unordered_map>

ManaCurveWidget::ManaCurveWidget(QWidget *parent, DeckListModel *_deckListModel)
    : QWidget(parent), deckListModel(_deckListModel)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    bannerWidget = new BannerWidget(this, tr("Mana Curve"), Qt::Vertical, 100);
    bannerWidget->setMaximumHeight(100);
    layout->addWidget(bannerWidget);

    barLayout = new QHBoxLayout(this);
    layout->addLayout(barLayout);

    connect(deckListModel, &DeckListModel::dataChanged, this, &ManaCurveWidget::analyzeManaCurve);

    retranslateUi();
}

void ManaCurveWidget::retranslateUi()
{
    bannerWidget->setText(tr("Mana Curve"));
}

void ManaCurveWidget::setDeckModel(DeckListModel *deckModel)
{
    deckListModel = deckModel;
    connect(deckListModel, &DeckListModel::dataChanged, this, &ManaCurveWidget::analyzeManaCurve);
    analyzeManaCurve();
}

std::unordered_map<int, int> ManaCurveWidget::analyzeManaCurve()
{
    manaCurveMap.clear();
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
                    int cmc = info->getCmc().toInt();
                    manaCurveMap[cmc]++;
                }
            }
        }
    }

    updateDisplay();

    return manaCurveMap;
}

void ManaCurveWidget::updateDisplay()
{
    // Clear the layout first
    if (barLayout != nullptr) {
        QLayoutItem *item;
        while ((item = barLayout->takeAt(0)) != nullptr) {
            item->widget()->deleteLater();
            delete item;
        }
    }

    int totalSum = 0;
    for (const auto &entry : manaCurveMap) {
        totalSum += entry.second;
    }

    // Convert unordered_map to ordered map to ensure sorting by CMC
    std::map<int, int> sortedManaCurve(manaCurveMap.begin(), manaCurveMap.end());

    // Add new widgets to the layout in sorted order
    for (const auto &entry : sortedManaCurve) {
        BarWidget *barWidget =
            new BarWidget(QString::number(entry.first), entry.second, totalSum, QColor(11, 11, 11), this);
        barLayout->addWidget(barWidget);
    }

    update(); // Update the widget display
}
