#include "deck_card_zone_display_widget.h"

#include "card_group_display_widgets/flat_card_group_display_widget.h"
#include "card_group_display_widgets/overlapped_card_group_display_widget.h"

#include <QResizeEvent>
#include <libcockatrice/card/card_info_comparator.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>

DeckCardZoneDisplayWidget::DeckCardZoneDisplayWidget(QWidget *parent,
                                                     DeckListModel *_deckListModel,
                                                     QPersistentModelIndex _trackedIndex,
                                                     QString _zoneName,
                                                     QString _activeGroupCriteria,
                                                     QStringList _activeSortCriteria,
                                                     DisplayType _displayType,
                                                     int bannerOpacity,
                                                     int subBannerOpacity,
                                                     CardSizeWidget *_cardSizeWidget)
    : QWidget(parent), deckListModel(_deckListModel), trackedIndex(_trackedIndex), zoneName(_zoneName),
      activeGroupCriteria(_activeGroupCriteria), activeSortCriteria(_activeSortCriteria), displayType(_displayType),
      bannerOpacity(bannerOpacity), subBannerOpacity(subBannerOpacity), cardSizeWidget(_cardSizeWidget)
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

    connect(deckListModel, &QAbstractItemModel::rowsInserted, this, &DeckCardZoneDisplayWidget::onCategoryAddition);
    connect(deckListModel, &QAbstractItemModel::rowsRemoved, this, &DeckCardZoneDisplayWidget::onCategoryRemoval);
}

void DeckCardZoneDisplayWidget::cleanupInvalidCardGroup(CardGroupDisplayWidget *displayWidget)
{
    cardGroupLayout->removeWidget(displayWidget);
    displayWidget->setParent(nullptr);
    for (auto idx : indexToWidgetMap.keys()) {
        if (!idx.isValid()) {
            indexToWidgetMap.remove(idx);
        }
    }
    delete displayWidget;
}

void DeckCardZoneDisplayWidget::constructAppropriateWidget(QPersistentModelIndex index)
{
    auto categoryName = deckListModel->data(index.sibling(index.row(), 1), Qt::EditRole).toString();
    if (indexToWidgetMap.contains(index)) {
        return;
    }
    if (displayType == DisplayType::Overlap) {
        auto *displayWidget = new OverlappedCardGroupDisplayWidget(
            cardGroupContainer, deckListModel, index, zoneName, categoryName, activeGroupCriteria, activeSortCriteria,
            subBannerOpacity, cardSizeWidget);
        connect(displayWidget, &OverlappedCardGroupDisplayWidget::cardClicked, this,
                &DeckCardZoneDisplayWidget::onClick);
        connect(displayWidget, &OverlappedCardGroupDisplayWidget::cardHovered, this,
                &DeckCardZoneDisplayWidget::onHover);
        connect(displayWidget, &CardGroupDisplayWidget::cleanupRequested, this,
                &DeckCardZoneDisplayWidget::cleanupInvalidCardGroup);
        connect(this, &DeckCardZoneDisplayWidget::activeSortCriteriaChanged, displayWidget,
                &CardGroupDisplayWidget::onActiveSortCriteriaChanged);
        cardGroupLayout->addWidget(displayWidget);
        indexToWidgetMap.insert(index, displayWidget);
    } else if (displayType == DisplayType::Flat) {
        auto *displayWidget =
            new FlatCardGroupDisplayWidget(cardGroupContainer, deckListModel, index, zoneName, categoryName,
                                           activeGroupCriteria, activeSortCriteria, subBannerOpacity, cardSizeWidget);
        connect(displayWidget, &FlatCardGroupDisplayWidget::cardClicked, this, &DeckCardZoneDisplayWidget::onClick);
        connect(displayWidget, &FlatCardGroupDisplayWidget::cardHovered, this, &DeckCardZoneDisplayWidget::onHover);
        connect(displayWidget, &CardGroupDisplayWidget::cleanupRequested, this,
                &DeckCardZoneDisplayWidget::cleanupInvalidCardGroup);
        connect(this, &DeckCardZoneDisplayWidget::activeSortCriteriaChanged, displayWidget,
                &CardGroupDisplayWidget::onActiveSortCriteriaChanged);
        cardGroupLayout->addWidget(displayWidget);
        indexToWidgetMap.insert(index, displayWidget);
    }
}

void DeckCardZoneDisplayWidget::displayCards()
{
    QSortFilterProxyModel proxy;
    proxy.setSourceModel(deckListModel);
    proxy.setSortRole(Qt::EditRole);
    proxy.sort(1, Qt::AscendingOrder);

    // 1. trackedIndex is a source index → map it to proxy space
    QModelIndex proxyParent = proxy.mapFromSource(trackedIndex);

    // 2. iterate children under the proxy parent
    for (int i = 0; i < proxy.rowCount(proxyParent); ++i) {
        QModelIndex proxyIndex = proxy.index(i, 0, proxyParent);

        // 3. map back to source
        QModelIndex sourceIndex = proxy.mapToSource(proxyIndex);

        // 4. persist the source index
        QPersistentModelIndex persistent(sourceIndex);

        constructAppropriateWidget(persistent);
    }
}

void DeckCardZoneDisplayWidget::onCategoryAddition(const QModelIndex &parent, int first, int last)
{
    if (!trackedIndex.isValid()) {
        emit requestCleanup(this);
        return;
    }
    if (parent == trackedIndex) {
        for (int i = first; i <= last; i++) {
            QPersistentModelIndex index = QPersistentModelIndex(deckListModel->index(i, 0, trackedIndex));

            constructAppropriateWidget(index);
        }
    }
}

void DeckCardZoneDisplayWidget::onCategoryRemoval(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);
    for (const QPersistentModelIndex &idx : indexToWidgetMap.keys()) {
        if (!idx.isValid()) {
            cardGroupLayout->removeWidget(indexToWidgetMap.value(idx));
            indexToWidgetMap.value(idx)->deleteLater();
            indexToWidgetMap.remove(idx);
        }
    }
    if (!trackedIndex.isValid()) {
        emit requestCleanup(this);
    }
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
void DeckCardZoneDisplayWidget::onHover(const ExactCard &card)
{
    emit cardHovered(card);
}

void DeckCardZoneDisplayWidget::refreshDisplayType(const DisplayType &_displayType)
{
    displayType = _displayType;
    QLayoutItem *item;
    while ((item = cardGroupLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        } else if (item->layout()) {
            item->layout()->deleteLater();
        }
        delete item;
    }

    indexToWidgetMap.clear();

    // We gotta wait for all the deleteLater's to finish so we fire after the next event cycle

    auto timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this]() { displayCards(); });
    timer->start();
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

    QList<ExactCard> cardsInZone = deckListModel->getCardsForZone(zoneName);

    for (const ExactCard &cardInZone : cardsInZone) {
        groupCriteriaValues.append(cardInZone.getInfo().getProperty(activeGroupCriteria));
    }

    groupCriteriaValues.removeDuplicates();
    groupCriteriaValues.sort();

    return groupCriteriaValues;
}
