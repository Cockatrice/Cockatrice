#include "deck_card_zone_display_widget.h"

#include "../../../../deck/deck_list_model.h"
#include "../../../../utility/card_info_comparator.h"
#include "overlapped_card_group_display_widget.h"

#include <QResizeEvent>

DeckCardZoneDisplayWidget::DeckCardZoneDisplayWidget(QWidget *parent,
                                                     DeckListModel *_deckListModel,
                                                     QString _zoneName,
                                                     QString _activeSortCriteria,
                                                     int bannerOpacity,
                                                     int subBannerOpacity)
    : QWidget(parent), deckListModel(_deckListModel), zoneName(_zoneName), activeSortCriteria(_activeSortCriteria),
      bannerOpacity(bannerOpacity), subBannerOpacity(subBannerOpacity)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    banner = new BannerWidget(this, zoneName, Qt::Orientation::Vertical, bannerOpacity);
    layout->addWidget(banner);

    cardGroupContainer = new QWidget(this);
    cardGroupLayout = new QVBoxLayout(cardGroupContainer);
    cardGroupContainer->setLayout(cardGroupLayout);
    layout->addWidget(cardGroupContainer);

    banner->setBuddy(cardGroupContainer);

    displayCards();
    connect(deckListModel, &DeckListModel::dataChanged, this, &DeckCardZoneDisplayWidget::displayCards);
}
void DeckCardZoneDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    for (QObject *child : layout->children()) {
        QWidget *widget = qobject_cast<QWidget *>(child);
        if (widget) {
            widget->setMaximumWidth(width());
        }
    }
}
void DeckCardZoneDisplayWidget::onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card)
{
    emit cardClicked(event, card, zoneName);
}
void DeckCardZoneDisplayWidget::onHover(CardInfoPtr card)
{
    emit cardHovered(card);
}

void DeckCardZoneDisplayWidget::displayCards()
{
    addCardGroupIfItDoesNotExist();
    deleteCardGroupIfItDoesNotExist();
}

void DeckCardZoneDisplayWidget::addCardGroupIfItDoesNotExist()
{
    QList<OverlappedCardGroupDisplayWidget *> cardGroupsDisplayWidgets =
        cardGroupContainer->findChildren<OverlappedCardGroupDisplayWidget *>();

    QList<QString> cardGroups = getSortCriteriaValueList();

    for (QString cardGroup : cardGroups) {
        bool found = false;
        for (OverlappedCardGroupDisplayWidget *cardGroupDisplayWidget : cardGroupsDisplayWidgets) {
            if (cardGroupDisplayWidget->cardGroupCategory == cardGroup) {
                found = true;
            }
        }

        if (!found) {
            OverlappedCardGroupDisplayWidget *display_widget = new OverlappedCardGroupDisplayWidget(
                this, deckListModel, zoneName, cardGroup, activeSortCriteria, subBannerOpacity);
            connect(display_widget, SIGNAL(cardClicked(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)), this,
                    SLOT(onClick(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)));
            connect(display_widget, SIGNAL(cardHovered(CardInfoPtr)), this, SLOT(onHover(CardInfoPtr)));
            cardGroupLayout->addWidget(display_widget);
        }
    }
}

void DeckCardZoneDisplayWidget::deleteCardGroupIfItDoesNotExist()
{
    QList<OverlappedCardGroupDisplayWidget *> cardGroupsDisplayWidgets =
        cardGroupLayout->findChildren<OverlappedCardGroupDisplayWidget *>();
    for (OverlappedCardGroupDisplayWidget *cardGroupDisplayWidget : cardGroupsDisplayWidgets) {
        bool found = false;
        for (QString cardGroup : getSortCriteriaValueList()) {
            if (cardGroupDisplayWidget->cardGroupCategory == cardGroup) {
                found = true;
            }
        }

        if (!found) {
            cardGroupLayout->removeWidget(cardGroupDisplayWidget);
        }
    }
}

void DeckCardZoneDisplayWidget::onActiveSortCriteriaChanged(QString _activeSortCriteria)
{
    activeSortCriteria = _activeSortCriteria;
}

QList<QString> DeckCardZoneDisplayWidget::getSortCriteriaValueList()
{
    QList<QString> sortCriteriaValues;

    QList<CardInfoPtr> *cardsInZone = deckListModel->getCardsAsCardInfoPtrsForZone(zoneName);

    for (CardInfoPtr cardInZone : *cardsInZone) {
        sortCriteriaValues.append(cardInZone->getProperty(activeSortCriteria));
    }

    sortCriteriaValues.removeDuplicates();
    sortCriteriaValues.sort();
    return sortCriteriaValues;
}
