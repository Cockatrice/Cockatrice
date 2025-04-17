#include "edhrec_commander_api_response_card_details_display_widget.h"

#include "../../../../../../game/cards/card_database_manager.h"
#include "../../tab_edhrec_main.h"

EdhrecCommanderApiResponseCardDetailsDisplayWidget::EdhrecCommanderApiResponseCardDetailsDisplayWidget(
    QWidget *parent,
    const EdhrecCommanderApiResponseCardDetails &_toDisplay)
    : QWidget(parent), toDisplay(_toDisplay)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    cardPictureWidget = new CardInfoPictureWidget(this);
    cardPictureWidget->setCard(CardDatabaseManager::getInstance()->guessCard(toDisplay.sanitized));

    label = new QLabel(this);
    label->setText(toDisplay.name + "\n" + toDisplay.label);
    label->setAlignment(Qt::AlignHCenter);

    int inclusionRate = 0;
    // Set label color based on inclusion rate
    if (toDisplay.potentialDecks != 0) {
        inclusionRate = (toDisplay.numDecks * 100) / toDisplay.potentialDecks;
    }

    QColor labelColor;
    if (inclusionRate <= 30) {
        labelColor = QColor(255, 0, 0); // Red
    } else if (inclusionRate <= 60) {
        int red = 255 - ((inclusionRate - 30) * 2);
        int green = (inclusionRate - 30) * 4; // Adjust green to make the transition smoother
        labelColor = QColor(red, green, 0);   // purple-ish
    } else if (inclusionRate <= 90) {
        int green = (inclusionRate - 60) * 5; // Increase green
        labelColor = QColor(100, green, 100); // Green shades
    } else {
        labelColor = QColor(100, 200, 100); // Dark Green
    }

    label->setStyleSheet(QString("color: %1").arg(labelColor.name()));

    layout->addWidget(cardPictureWidget);
    layout->addWidget(label);

    QWidget *currentParent = parentWidget();
    TabEdhRecMain *parentTab = nullptr;

    while (currentParent) {
        if ((parentTab = qobject_cast<TabEdhRecMain *>(currentParent))) {
            break;
        }
        currentParent = currentParent->parentWidget();
    }

    if (parentTab) {
        connect(cardPictureWidget, &CardInfoPictureWidget::cardClicked, this,
                &EdhrecCommanderApiResponseCardDetailsDisplayWidget::actRequestPageNavigation);
        connect(this, &EdhrecCommanderApiResponseCardDetailsDisplayWidget::requestUrl, parentTab,
                &TabEdhRecMain::actNavigatePage);
    }
}

void EdhrecCommanderApiResponseCardDetailsDisplayWidget::actRequestPageNavigation()
{
    emit requestUrl(toDisplay.url);
}
