#include "visual_deck_editor_sample_hand_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../cards/card_info_picture_widget.h"
#include "../cards/card_size_widget.h"
#include "../deck_analytics/analyzer_modules/draw_probability/draw_probability_widget.h"
#include "../general/layout_containers/flow_widget.h"
#include "deck_list_model.h"
#include "sample_hand_generator.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/settings/cards_display_settings.h>
#include <libcockatrice/utility/qt_utils.h>
#include <random>

class DeckListStatisticsAnalyzer;

VisualDeckEditorSampleHandWidget::VisualDeckEditorSampleHandWidget(QWidget *parent,
                                                                   DeckListModel *_deckListModel,
                                                                   DeckListStatisticsAnalyzer *_statsAnalyzer)
    : QWidget(parent), deckListModel(_deckListModel), statsAnalyzer(_statsAnalyzer)
{
    layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    setLayout(layout);

    auto upperLayout = QtUtils::makeLayout<QVBoxLayout>(nullptr, 0);

    resetAndHandSizeContainerWidget = new QWidget(this);
    resetAndHandSizeLayout = new QHBoxLayout(resetAndHandSizeContainerWidget);
    resetAndHandSizeLayout->setContentsMargins(11, 0, 11, 0);
    resetAndHandSizeContainerWidget->setLayout(resetAndHandSizeLayout);

    resetButton = new QPushButton(this);
    connect(resetButton, SIGNAL(clicked()), this, SLOT(updateDisplay()));
    resetAndHandSizeLayout->addWidget(resetButton);

    handSizeSpinBox = new QSpinBox(this);
    handSizeSpinBox->setMinimum(1);
    handSizeSpinBox->setValue(SettingsCache::instance().cardsDisplay().getSampleHandSize());
    connect(handSizeSpinBox, qOverload<int>(&QSpinBox::valueChanged), &SettingsCache::instance().cardsDisplay(),
            &CardsDisplaySettings::setSampleHandSize);
    connect(handSizeSpinBox, qOverload<int>(&QSpinBox::valueChanged), this,
            &VisualDeckEditorSampleHandWidget::updateDisplay);
    resetAndHandSizeLayout->addWidget(handSizeSpinBox);

    upperLayout->addWidget(resetAndHandSizeContainerWidget);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    upperLayout->addWidget(flowWidget);

    cardSizeWidget = new CardSizeWidget(this, flowWidget);
    cardSizeWidget->enableCtrlScrollResize(flowWidget);
    upperLayout->addWidget(cardSizeWidget);

    auto upperLayoutWidget = new QWidget(this);
    upperLayoutWidget->setLayout(upperLayout);

    drawProbabilityWidget = new DrawProbabilityWidget(this, statsAnalyzer);

    auto *splitter = new QSplitter(this);
    splitter->setObjectName("splitter");
    splitter->setOrientation(Qt::Vertical);

    splitter->addWidget(upperLayoutWidget);
    splitter->addWidget(drawProbabilityWidget);

    layout->addWidget(splitter);

    retranslateUi();
}

void VisualDeckEditorSampleHandWidget::retranslateUi()
{
    resetButton->setText(tr("Draw a new sample hand"));
    handSizeSpinBox->setToolTip(tr("Sample hand size"));
}

void VisualDeckEditorSampleHandWidget::setDeckModel(DeckListModel *deckModel)
{
    deckListModel = deckModel;
    updateDisplay();
}

void VisualDeckEditorSampleHandWidget::updateDisplay()
{
    flowWidget->clearLayout();
    for (const ExactCard &card : getRandomCards(handSizeSpinBox->value())) {
        auto displayWidget = new CardInfoPictureWidget(this);
        displayWidget->setCard(card);
        displayWidget->setToolTip(card.getName());
        displayWidget->setAccessibleName(card.getName());
        displayWidget->setScaleFactor(cardSizeWidget->getSlider()->value());
        connect(cardSizeWidget->getSlider(), &QSlider::valueChanged, displayWidget,
                &CardInfoPictureWidget::setScaleFactor);
        flowWidget->addWidget(displayWidget);
    }
}

QList<ExactCard> VisualDeckEditorSampleHandWidget::getRandomCards(int amountToGet)
{
    if (!deckListModel || !deckListModel->getDeckList()) {
        return {};
    }

    std::random_device seed;
    std::mt19937 random(seed());
    return generateSampleHand(*deckListModel->getDeckList(), *CardDatabaseManager::query(), amountToGet, random);
}
