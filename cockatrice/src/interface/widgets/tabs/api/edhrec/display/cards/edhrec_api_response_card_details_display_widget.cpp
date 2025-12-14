#include "edhrec_api_response_card_details_display_widget.h"

#include "../../../../../general/display/background_plate_widget.h"
#include "../../tab_edhrec_main.h"

#include <libcockatrice/card/database/card_database_manager.h>

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

    inclusionDisplayWidget = new EdhrecApiResponseCardInclusionDisplayWidget(this, toDisplay);

    synergyDisplayWidget = new EdhrecApiResponseCardSynergyDisplayWidget(this, toDisplay);

    layout->addWidget(nameLabel);
    layout->addWidget(cardPictureWidget);

    backgroundPlateWidget = new BackgroundPlateWidget(this);
    auto plateLayout = new QVBoxLayout(backgroundPlateWidget);

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
                &EdhrecApiResponseCardDetailsDisplayWidget::actRequestPageNavigation);
        connect(parentTab->getCardSizeSlider()->getSlider(), &QSlider::valueChanged, cardPictureWidget,
                &CardInfoPictureWidget::setScaleFactor);
        connect(this, &EdhrecApiResponseCardDetailsDisplayWidget::requestUrl, parentTab,
                &TabEdhRecMain::actNavigatePage);
    }
}

void EdhrecApiResponseCardDetailsDisplayWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    actRequestPageNavigation();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void EdhrecApiResponseCardDetailsDisplayWidget::enterEvent(QEnterEvent *event)
#else
void EdhrecApiResponseCardDetailsDisplayWidget::enterEvent(QEvent *event)
#endif
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
