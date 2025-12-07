#include "edhrec_api_response_commander_details_display_widget.h"

#include "../../../../../cards/card_info_picture_widget.h"
#include "../../tab_edhrec_main.h"
#include "../card_prices/edhrec_api_response_card_prices_display_widget.h"
#include "edhrec_commander_api_response_bracket_navigation_widget.h"

#include <libcockatrice/card/database/card_database_manager.h>

EdhrecCommanderResponseCommanderDetailsDisplayWidget::EdhrecCommanderResponseCommanderDetailsDisplayWidget(
    QWidget *parent,
    const EdhrecCommanderApiResponseCommanderDetails &_commanderDetails,
    QString baseUrl)
    : QWidget(parent), commanderDetails(_commanderDetails)
{
    layout = new QHBoxLayout(this);
    setLayout(layout);

    commanderLayout = new QHBoxLayout();
    commanderDetailsLayout = new QVBoxLayout();
    commanderDetailsLayout->setAlignment(Qt::AlignCenter);
    navigationAndPricesLayout = new QVBoxLayout();

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

    commanderName = new QLabel(this);
    commanderName->setText(commanderDetails.getName());
    commanderName->setAlignment(Qt::AlignCenter);
    commanderName->setStyleSheet("font-size: 28px; font-weight: bold");

    label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 16px");

    salt = new QLabel(this);
    salt->setAlignment(Qt::AlignCenter);
    salt->setStyleSheet("font-size: 16px");

    cardPricesDisplayWidget = new EdhrecApiResponseCardPricesDisplayWidget(this, commanderDetails.getPrices());

    navigationWidget = new EdhrecCommanderApiResponseNavigationWidget(this, commanderDetails, baseUrl);

    commanderLayout->addWidget(commanderPicture);
    commanderDetailsLayout->addWidget(commanderName);
    commanderDetailsLayout->addSpacing(1);
    commanderDetailsLayout->addWidget(label);
    commanderDetailsLayout->addWidget(salt);
    commanderDetailsLayout->addWidget(cardPricesDisplayWidget);
    commanderLayout->addLayout(commanderDetailsLayout);
    navigationAndPricesLayout->addWidget(navigationWidget);
    // navigationAndPricesLayout->addWidget(cardPricesDisplayWidget);

    layout->addLayout(commanderLayout);
    layout->addLayout(navigationAndPricesLayout);

    retranslateUi();
}

void EdhrecCommanderResponseCommanderDetailsDisplayWidget::retranslateUi()
{
    label->setText(commanderDetails.getLabel());
    salt->setText(tr("Salt: ") + QString::number(commanderDetails.getSalt()));
}
