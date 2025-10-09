#include "edhrec_api_response_commander_details_display_widget.h"

#include "../../../../../cards/card_info_picture_widget.h"
#include "../../tab_edhrec_main.h"
#include "../card_prices/edhrec_api_response_card_prices_display_widget.h"

#include <libcockatrice/card/database/card_database_manager.h>

EdhrecCommanderResponseCommanderDetailsDisplayWidget::EdhrecCommanderResponseCommanderDetailsDisplayWidget(
    QWidget *parent,
    const EdhrecCommanderApiResponseCommanderDetails &_commanderDetails,
    QString baseUrl)
    : QWidget(parent), commanderDetails(_commanderDetails)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    commanderPicture = new CardInfoPictureWidget(this);
    commanderPicture->setCard(CardDatabaseManager::query()->getCard({commanderDetails.getName()}));

    QWidget *currentParent = parentWidget();
    TabEdhRecMain *parentTab = nullptr;

    while (currentParent) {
        if ((parentTab = qobject_cast<TabEdhRecMain *>(currentParent))) {
            break;
        }
        currentParent = currentParent->parentWidget();
    }

    if (parentTab) {
        connect(parentTab->getCardSizeSlider()->getSlider(), &QSlider::valueChanged, commanderPicture,
                &CardInfoPictureWidget::setScaleFactor);
        commanderPicture->setScaleFactor(parentTab->getCardSizeSlider()->getSlider()->value());
    }

    commanderDetails.debugPrint();

    label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
    salt = new QLabel(this);
    salt->setAlignment(Qt::AlignCenter);

    cardPricesDisplayWidget = new EdhrecApiResponseCardPricesDisplayWidget(this, commanderDetails.getPrices());

    navigationWidget = new EdhrecCommanderApiResponseNavigationWidget(this, commanderDetails, baseUrl);

    layout->addWidget(commanderPicture);
    layout->addWidget(label);
    layout->addWidget(salt);
    layout->addWidget(cardPricesDisplayWidget);
    layout->addWidget(navigationWidget);

    retranslateUi();
}

void EdhrecCommanderResponseCommanderDetailsDisplayWidget::retranslateUi()
{
    label->setText(commanderDetails.getLabel());
    salt->setText(tr("Salt: ") + QString::number(commanderDetails.getSalt()));
}
