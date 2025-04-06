#include "deck_card_zone_display_widget.h"

#include "../../../../deck/deck_list_model.h"
#include "../../../../utility/card_info_comparator.h"
#include "card_group_display_widgets/flat_card_group_display_widget.h"
#include "card_group_display_widgets/overlapped_card_group_display_widget.h"

#include <QResizeEvent>

DeckCardZoneDisplayWidget::DeckCardZoneDisplayWidget(QWidget *parent,
                                                     DeckListModel *_deckListModel,
                                                     QString _zoneName,
                                                     QString _activeGroupCriteria,
                                                     QStringList _activeSortCriteria,
                                                     int bannerOpacity,
                                                     int subBannerOpacity,
                                                     CardSizeWidget *_cardSizeWidget)
    : QWidget(parent), deckListModel(_deckListModel), zoneName(_zoneName), activeGroupCriteria(_activeGroupCriteria),
      activeSortCriteria(_activeSortCriteria), bannerOpacity(bannerOpacity), subBannerOpacity(subBannerOpacity),
      cardSizeWidget(_cardSizeWidget)
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

void DeckCardZoneDisplayWidget::refreshDisplayType(const QString &_displayType)
{
    displayType = _displayType;
    QLayoutItem *item;
    while ((item = cardGroupLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        } else if (item->layout()) {
            delete item->layout();
        }
        delete item;
    }

    // We gotta wait for all the deleteLater's to finish so we fire after the next event cycle

    auto timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this]() { displayCards(); });
    timer->start();
}

void DeckCardZoneDisplayWidget::addCardGroupIfItDoesNotExist()
{
    QList<CardGroupDisplayWidget *> cardGroupsDisplayWidgets =
        cardGroupContainer->findChildren<CardGroupDisplayWidget *>();

    QList<QString> cardGroups = getGroupCriteriaValueList();

    for (QString cardGroup : cardGroups) {
        bool found = false;
        for (CardGroupDisplayWidget *cardGroupDisplayWidget : cardGroupsDisplayWidgets) {
            if (cardGroupDisplayWidget->cardGroupCategory == cardGroup) {
                found = true;
            }
        }

        if (!found) {
            if (displayType == "overlap") {
                auto *display_widget = new OverlappedCardGroupDisplayWidget(
                    cardGroupContainer, deckListModel, zoneName, cardGroup, activeGroupCriteria, activeSortCriteria,
                    subBannerOpacity, cardSizeWidget);
                connect(display_widget, SIGNAL(cardClicked(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)),
                        this, SLOT(onClick(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)));
                connect(display_widget, SIGNAL(cardHovered(CardInfoPtr)), this, SLOT(onHover(CardInfoPtr)));
                connect(this, &DeckCardZoneDisplayWidget::activeSortCriteriaChanged, display_widget,
                        &CardGroupDisplayWidget::onActiveSortCriteriaChanged);
                cardGroupLayout->addWidget(display_widget);
            } else if (displayType == "flat") {
                auto *display_widget = new FlatCardGroupDisplayWidget(
                    cardGroupContainer, deckListModel, zoneName, cardGroup, activeGroupCriteria, activeSortCriteria,
                    subBannerOpacity, cardSizeWidget);
                connect(display_widget, SIGNAL(cardClicked(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)),
                        this, SLOT(onClick(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)));
                connect(display_widget, SIGNAL(cardHovered(CardInfoPtr)), this, SLOT(onHover(CardInfoPtr)));
                connect(this, &DeckCardZoneDisplayWidget::activeSortCriteriaChanged, display_widget,
                        &CardGroupDisplayWidget::onActiveSortCriteriaChanged);
                cardGroupLayout->addWidget(display_widget);
            }
        }
    }
}

void DeckCardZoneDisplayWidget::deleteCardGroupIfItDoesNotExist()
{
    QList<CardGroupDisplayWidget *> cardGroupsDisplayWidgets =
        cardGroupContainer->findChildren<CardGroupDisplayWidget *>();

    QList<QString> validGroups = getGroupCriteriaValueList();

    for (CardGroupDisplayWidget *cardGroupDisplayWidget : cardGroupsDisplayWidgets) {
        if (!validGroups.contains(cardGroupDisplayWidget->cardGroupCategory)) {
            cardGroupLayout->removeWidget(cardGroupDisplayWidget);
            cardGroupDisplayWidget->deleteLater(); // Properly delete the widget after the event loop cycles
        } else {
        }
    }
}

void DeckCardZoneDisplayWidget::onActiveGroupCriteriaChanged(QString _activeGroupCriteria)
{
    activeGroupCriteria = _activeGroupCriteria;
    displayCards();
}

void DeckCardZoneDisplayWidget::onActiveSortCriteriaChanged(QStringList _activeSortCriteria)
{
    activeSortCriteria = _activeSortCriteria;
    emit activeSortCriteriaChanged(activeSortCriteria);
}

QList<QString> DeckCardZoneDisplayWidget::getGroupCriteriaValueList()
{
    QList<QString> groupCriteriaValues;

    QList<CardInfoPtr> cardsInZone = deckListModel->getCardsAsCardInfoPtrsForZone(zoneName);

    for (CardInfoPtr cardInZone : cardsInZone) {
        groupCriteriaValues.append(cardInZone->getProperty(activeGroupCriteria));
    }

    groupCriteriaValues.removeDuplicates();
    groupCriteriaValues.sort();

    return groupCriteriaValues;
}
