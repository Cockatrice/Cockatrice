#include "deck_card_zone_display_widget.h"

#include "../../../../deck/deck_list_model.h"
#include "../../../../utility/card_info_comparator.h"
#include "overlapped_card_group_display_widget.h"

#include <QResizeEvent>

DeckCardZoneDisplayWidget::DeckCardZoneDisplayWidget(QWidget *parent,
                                                     DeckListModel *_deckListModel,
                                                     QString _zoneName,
                                                     QString _activeGroupCriteria,
                                                     QString _activeSortCriteria,
                                                     int bannerOpacity,
                                                     int subBannerOpacity)
    : QWidget(parent), deckListModel(_deckListModel), zoneName(_zoneName), activeGroupCriteria(_activeGroupCriteria),
      activeSortCriteria(_activeSortCriteria), bannerOpacity(bannerOpacity), subBannerOpacity(subBannerOpacity)
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

    QList<QString> cardGroups = getGroupCriteriaValueList();

    for (QString cardGroup : cardGroups) {
        bool found = false;
        for (OverlappedCardGroupDisplayWidget *cardGroupDisplayWidget : cardGroupsDisplayWidgets) {
            if (cardGroupDisplayWidget->cardGroupCategory == cardGroup) {
                found = true;
            }
        }

        if (!found) {
            OverlappedCardGroupDisplayWidget *display_widget =
                new OverlappedCardGroupDisplayWidget(cardGroupContainer, deckListModel, zoneName, cardGroup,
                                                     activeGroupCriteria, activeSortCriteria, subBannerOpacity);
            connect(display_widget, SIGNAL(cardClicked(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)), this,
                    SLOT(onClick(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)));
            connect(display_widget, SIGNAL(cardHovered(CardInfoPtr)), this, SLOT(onHover(CardInfoPtr)));
            connect(this, &DeckCardZoneDisplayWidget::activeSortCriteriaChanged, display_widget,
                    &OverlappedCardGroupDisplayWidget::onActiveSortCriteriaChanged);
            cardGroupLayout->addWidget(display_widget);
        }
    }
}

void DeckCardZoneDisplayWidget::deleteCardGroupIfItDoesNotExist()
{
    QList<OverlappedCardGroupDisplayWidget *> cardGroupsDisplayWidgets =
        cardGroupContainer->findChildren<OverlappedCardGroupDisplayWidget *>();

    QList<QString> validGroups = getGroupCriteriaValueList();

    qDebug() << "DeckCardZoneDisplayWidget: Checking for obsolete card groups.";
    qDebug() << "Valid groups (should remain):" << validGroups;

    for (OverlappedCardGroupDisplayWidget *cardGroupDisplayWidget : cardGroupsDisplayWidgets) {
        qDebug() << "Checking group:" << cardGroupDisplayWidget->cardGroupCategory;

        if (!validGroups.contains(cardGroupDisplayWidget->cardGroupCategory)) {
            qDebug() << "Deleting group:" << cardGroupDisplayWidget->cardGroupCategory;
            cardGroupLayout->removeWidget(cardGroupDisplayWidget);
            cardGroupDisplayWidget->deleteLater(); // Properly delete the widget after the event loop cycles
        } else {
            qDebug() << "Keeping group:" << cardGroupDisplayWidget->cardGroupCategory;
        }
    }

    qDebug() << "Finished checking for obsolete card groups.";
}

void DeckCardZoneDisplayWidget::onActiveGroupCriteriaChanged(QString _activeGroupCriteria)
{
    activeGroupCriteria = _activeGroupCriteria;
    qDebug() << "DeckCardZoneDisplayWidget says: We group by " << activeGroupCriteria << " and display cards";
    displayCards();
}

void DeckCardZoneDisplayWidget::onActiveSortCriteriaChanged(QString _activeSortCriteria)
{
    activeSortCriteria = _activeSortCriteria;
    qDebug() << "DeckCardZoneDisplayWidget says: we sort by" << activeSortCriteria << " and emit a signal";
    emit activeSortCriteriaChanged(activeSortCriteria);
}

QList<QString> DeckCardZoneDisplayWidget::getGroupCriteriaValueList()
{
    QList<QString> groupCriteriaValues;

    QList<CardInfoPtr> *cardsInZone = deckListModel->getCardsAsCardInfoPtrsForZone(zoneName);

    for (CardInfoPtr cardInZone : *cardsInZone) {
        groupCriteriaValues.append(cardInZone->getProperty(activeGroupCriteria));
    }

    groupCriteriaValues.removeDuplicates();
    groupCriteriaValues.sort();

    qDebug() << "These are the group by criteria values";
    for (QString groupCriteria : groupCriteriaValues) {
        qDebug() << groupCriteria;
    }

    return groupCriteriaValues;
}
