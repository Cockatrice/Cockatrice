#include "edhrec_api_response_card_inclusion_display_widget.h"

EdhrecApiResponseCardInclusionDisplayWidget::EdhrecApiResponseCardInclusionDisplayWidget(
    QWidget *parent,
    const EdhrecApiResponseCardDetails &_toDisplay)
    : QWidget(parent), toDisplay(_toDisplay)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    amountLabel = new QLabel(this);
    amountLabel->setAlignment(Qt::AlignCenter);
    inclusionLabel = new QLabel(this);
    inclusionLabel->setAlignment(Qt::AlignCenter);
    percentBarWidget = new PercentBarWidget(this, toDisplay.inclusion / (toDisplay.potentialDecks / 100.0));

    if (toDisplay.inclusion != 0 && toDisplay.potentialDecks != 0) {
        layout->addWidget(amountLabel);
        layout->addWidget(inclusionLabel);
        layout->addWidget(percentBarWidget);
    } else {
        hide();
    }

    retranslateUi();
}

void EdhrecApiResponseCardInclusionDisplayWidget::retranslateUi()
{
    amountLabel->setText(tr("In %1 decks").arg(QString::number(toDisplay.inclusion)));
    inclusionLabel->setText(tr("%1% of %2 decks")
                                .arg(QString::number(toDisplay.inclusion / (toDisplay.potentialDecks / 100.0), 'f', 2),
                                     QString::number(toDisplay.potentialDecks)));
}
