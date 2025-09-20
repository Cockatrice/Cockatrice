#include "edhrec_api_response_card_synergy_display_widget.h"

EdhrecApiResponseCardSynergyDisplayWidget::EdhrecApiResponseCardSynergyDisplayWidget(
    QWidget *parent,
    const EdhrecApiResponseCardDetails &_toDisplay)
    : QWidget(parent), toDisplay(_toDisplay)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
    percentBarWidget = new PercentBarWidget(this, toDisplay.synergy * 100.0);

    if (toDisplay.synergy != 0) {
        layout->addWidget(label);
        layout->addWidget(percentBarWidget);
    } else {
        hide();
    }

    retranslateUi();
}

void EdhrecApiResponseCardSynergyDisplayWidget::retranslateUi()
{
    label->setText(tr("%1% Synergy").arg(QString::number(toDisplay.synergy * 100.0, 'f', 1)));
}
