#include "edhrec_commander_api_response_commander_details_display_widget.h"

#include "../../../../../../game/cards/card_database_manager.h"
#include "../../../../../ui/widgets/cards/card_info_picture_widget.h"
#include "../../tab_edhrec_main.h"

#include <QLabel>
#include <QPushButton>

EdhrecCommanderResponseCommanderDetailsDisplayWidget::EdhrecCommanderResponseCommanderDetailsDisplayWidget(
    QWidget *parent,
    const EdhrecCommanderApiResponseCommanderDetails &_commanderDetails)
    : QWidget(parent), commanderDetails(_commanderDetails)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    commanderPicture = new CardInfoPictureWidget(this);
    commanderPicture->setCard(CardDatabaseManager::getInstance()->getCard(commanderDetails.getName()));

    commanderDetails.debugPrint();

    label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
    salt = new QLabel(this);
    salt->setAlignment(Qt::AlignCenter);

    comboPushButton = new QPushButton(this);

    layout->addWidget(commanderPicture);
    layout->addWidget(label);
    layout->addWidget(salt);
    layout->addWidget(comboPushButton);

    QWidget *currentParent = parentWidget();
    TabEdhRecMain *parentTab = nullptr;

    while (currentParent) {
        if ((parentTab = qobject_cast<TabEdhRecMain *>(currentParent))) {
            break;
        }
        currentParent = currentParent->parentWidget();
    }

    if (parentTab) {
        connect(comboPushButton, &QPushButton::clicked, this,
                &EdhrecCommanderResponseCommanderDetailsDisplayWidget::actRequestComboNavigation);
        connect(this, &EdhrecCommanderResponseCommanderDetailsDisplayWidget::requestUrl, parentTab,
                &TabEdhRecMain::actNavigatePage);
    }

    retranslateUi();
}

void EdhrecCommanderResponseCommanderDetailsDisplayWidget::retranslateUi()
{
    label->setText(commanderDetails.getLabel());
    salt->setText(tr("Salt: ") + QString::number(commanderDetails.getSalt()));
    comboPushButton->setText(tr("Combos"));
}

void EdhrecCommanderResponseCommanderDetailsDisplayWidget::actRequestComboNavigation()
{
    emit requestUrl("/combos/" + commanderDetails.getSanitized());
}