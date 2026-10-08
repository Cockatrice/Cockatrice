#include "edhrec_api_response_card_details_display_widget.h"

#include "../../../../../cards/card_info_picture_widget.h"
#include "../../../../../cards/card_size_widget.h"
#include "../../../../../general/display/background_plate_widget.h"
#include "../../api_response/cards/edhrec_api_response_card_details.h"
#include "../../tab_edhrec_main.h"
#include "edhrec_api_response_card_inclusion_display_widget.h"
#include "edhrec_api_response_card_synergy_display_widget.h"
#include "libcockatrice/card/database/card_database_querier.h"

#include <QLabel>
#include <QMouseEvent>
#include <QObject>
#include <QSlider>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_manager.h>
#include <qnamespace.h>

EdhrecApiResponseCardDetailsDisplayWidget::EdhrecApiResponseCardDetailsDisplayWidget(
    QWidget *parent,
    const EdhrecApiResponseCardDetails &_toDisplay)
    : QWidget(parent), toDisplay(_toDisplay)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    cardPictureWidget = new CardInfoPictureWidget(this);
    cardPictureWidget->setCard(CardDatabaseManager::query()->guessCard({toDisplay.sanitized}));

    nameLabel = new QLabel(this);
    nameLabel->setText(toDisplay.name);
    nameLabel->setAlignment(Qt::AlignHCenter);
    nameLabel->setStyleSheet("font-size: 20px; font-weight: bold");

    inclusionDisplayWidget = new EdhrecApiResponseCardInclusionDisplayWidget(this, toDisplay);

    synergyDisplayWidget = new EdhrecApiResponseCardSynergyDisplayWidget(this, toDisplay);

    backgroundPlateWidget = new BackgroundPlateWidget(this);
    auto plateLayout = new QVBoxLayout(backgroundPlateWidget);

    plateLayout->addWidget(nameLabel);
    plateLayout->addWidget(cardPictureWidget);
    plateLayout->addWidget(inclusionDisplayWidget);
    plateLayout->addWidget(synergyDisplayWidget);

    layout->addWidget(backgroundPlateWidget);

    QWidget *currentParent = parentWidget();
    TabEdhRecMain *parentTab = nullptr;

    while (currentParent) {
        if ((parentTab = qobject_cast<TabEdhRecMain *>(currentParent))) {
            break;
        }
        currentParent = currentParent->parentWidget();
    }

    if (parentTab) {
        cardPictureWidget->setScaleFactor(parentTab->getCardSizeSlider()->getSlider()->value());
        connect(cardPictureWidget, &CardInfoPictureWidget::cardClicked, this,
                &EdhrecApiResponseCardDetailsDisplayWidget::mousePressEvent);
        connect(parentTab->getCardSizeSlider()->getSlider(), &QSlider::valueChanged, cardPictureWidget,
                &CardInfoPictureWidget::setScaleFactor);
        connect(this, &EdhrecApiResponseCardDetailsDisplayWidget::requestUrl, parentTab,
                &TabEdhRecMain::actNavigatePage);
        parentTab->getCardSizeSlider()->enableCtrlScrollResize(this);
    }
}

void EdhrecApiResponseCardDetailsDisplayWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
        actRequestPageNavigation();
    }
}

void EdhrecApiResponseCardDetailsDisplayWidget::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);
    backgroundPlateWidget->setFocused(true);
}

void EdhrecApiResponseCardDetailsDisplayWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    backgroundPlateWidget->setFocused(false);
}

void EdhrecApiResponseCardDetailsDisplayWidget::actRequestPageNavigation()
{
    emit requestUrl(toDisplay.url);
}
