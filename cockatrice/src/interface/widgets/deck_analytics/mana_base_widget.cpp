#include "mana_base_widget.h"

#include "../general/display/banner_widget.h"
#include "../general/display/bar_widget.h"

#include <QHash>
#include <QRegularExpression>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/deck_loader.h>

ManaBaseWidget::ManaBaseWidget(QWidget *parent, DeckListModel *_deckListModel)
    : QWidget(parent), deckListModel(_deckListModel)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    bannerWidget = new BannerWidget(this, tr("Mana Base"), Qt::Vertical, 100);
    bannerWidget->setMaximumHeight(100);
    layout->addWidget(bannerWidget);

    barContainer = new QWidget(this);
    barLayout = new QHBoxLayout(barContainer);
    layout->addWidget(barContainer);

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

    int highestEntry = 0;
    for (auto entry : manaBaseMap) {
        if (entry > highestEntry) {
            highestEntry = entry;
        }
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
        QColor barColor = manaColors.value(manaColor, Qt::gray);
        BarWidget *barWidget = new BarWidget(QString(manaColor), manaBaseMap[manaColor], highestEntry, barColor, this);
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
                CardInfoPtr info = CardDatabaseManager::query()->getCardInfo(currentCard->getName());
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

void ManaBaseWidget::mergeManaCounts(QHash<QString, int> &manaCounts1, const QHash<QString, int> &manaCounts2)
{
    for (auto it = manaCounts2.constBegin(); it != manaCounts2.constEnd(); ++it) {
        manaCounts1[it.key()] += it.value();
    }
}
