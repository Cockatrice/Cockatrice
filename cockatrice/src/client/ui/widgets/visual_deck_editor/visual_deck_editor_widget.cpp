#include "visual_deck_editor_widget.h"

#include "../../../../deck/deck_list_model.h"
#include "../../../../deck/deck_loader.h"
#include "../../../../game/cards/card_database.h"
#include "../../../../game/cards/card_database_manager.h"
#include "../../../../main.h"
#include "../../../../utility/card_info_comparator.h"
#include "../../layouts/overlap_layout.h"
#include "../cards/card_info_picture_with_text_overlay_widget.h"
#include "../cards/deck_card_zone_display_widget.h"
#include "../general/layout_containers/flow_widget.h"
#include "../general/layout_containers/overlap_control_widget.h"

#include <QHBoxLayout>
#include <QResizeEvent>
#include <qscrollarea.h>

VisualDeckEditorWidget::VisualDeckEditorWidget(QWidget *parent, DeckListModel *_deckListModel)
    : QWidget(parent), deckListModel(_deckListModel)
{
    this->mainDeckCards = new QList<CardInfoPtr>;
    this->sideboardCards = new QList<CardInfoPtr>;
    connect(deckListModel, &DeckListModel::dataChanged, this, &VisualDeckEditorWidget::decklistDataChanged);

    // The Main Widget and Main Layout, which contain a single Widget: The Scroll Area
    this->setMinimumSize(0, 0);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);

    sortByComboBox = new QComboBox();
    QStringList sortProperties;
    sortProperties << "maintype"
                   << "colors"
                   << "cmc"
                   << "name";
    sortByComboBox->addItems(sortProperties);
    connect(sortByComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(activeSortCriteriaChanged()));

    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumSize(0, 0);

    // Set scrollbar policies
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    zoneContainer = new QWidget(scrollArea);
    zoneContainerLayout = new QVBoxLayout(zoneContainer);
    zoneContainer->setLayout(zoneContainerLayout);
    scrollArea->addScrollBarWidget(zoneContainer, Qt::AlignHCenter);
    scrollArea->setWidget(zoneContainer);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);

    overlap_control_widget = new OverlapControlWidget(80, 1, 1, Qt::Vertical, this);

    mainLayout->addWidget(sortByComboBox);
    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(overlap_control_widget);
}

void VisualDeckEditorWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    zoneContainer->setMaximumWidth(scrollArea->viewport()->width());
}

void VisualDeckEditorWidget::activeSortCriteriaChanged()
{
    activeSortCriteria = sortByComboBox->currentText();
    qDebug() << "activeSortCriteria changed: " << activeSortCriteria;
    updateDisplay();
}

void VisualDeckEditorWidget::decklistDataChanged(QModelIndex topLeft, QModelIndex bottomRight)
{
    // Might use these at some point.
    (void)topLeft;
    (void)bottomRight;
    // Necessary to delay this in this manner else the updateDisplay will nuke widgets while their onClick event
    // hasn't returned yet. Interval of 0 means QT will schedule this after the current event loop has finished.
    QTimer::singleShot(0, this, [this] { updateDisplay(); });
}

void VisualDeckEditorWidget::updateDisplay()
{
    // Clear the layout first
    populateCards();
    zoneContainer = new QWidget(scrollArea);
    zoneContainer->setMaximumWidth(scrollArea->viewport()->width());
    zoneContainerLayout = new QVBoxLayout(zoneContainer);

    DeckCardZoneDisplayWidget *mainBoardWidget =
        new DeckCardZoneDisplayWidget(zoneContainer, sortCards(mainDeckCards), tr("Mainboard"), 20, 10);
    connect(mainBoardWidget, SIGNAL(cardHovered(CardInfoPtr)), this, SLOT(onHover(CardInfoPtr)));
    connect(mainBoardWidget, SIGNAL(cardClicked(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)), this,
            SLOT(onMainboardClick(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)));

    DeckCardZoneDisplayWidget *sideBoardWidget =
        new DeckCardZoneDisplayWidget(zoneContainer, sortCards(sideboardCards), tr("Sideboard"), 40, 30);
    connect(sideBoardWidget, SIGNAL(cardHovered(CardInfoPtr)), this, SLOT(onHover(CardInfoPtr)));
    connect(sideBoardWidget, SIGNAL(cardClicked(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)), this,
            SLOT(onSideboardClick(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)));

    zoneContainerLayout->addWidget(mainBoardWidget);
    zoneContainerLayout->addWidget(sideBoardWidget);

    scrollArea->setWidget(zoneContainer);
    update();
}

QList<QPair<QString, QList<CardInfoPtr>>> VisualDeckEditorWidget::sortCards(QList<CardInfoPtr> *cardsToSort)
{
    QList<QPair<QString, QList<CardInfoPtr>>> sortedCardGroups;
    QList<CardInfoPtr> activeList;

    QStringList sortCriteria;
    sortCriteria.append(activeSortCriteria);
    sortCardList(sortCriteria, Qt::SortOrder::AscendingOrder);

    QString lastSortCriteriaValue = cardsToSort->isEmpty() ? "" : cardsToSort->at(0)->getProperty(activeSortCriteria);

    for (int i = 0; i < cardsToSort->size(); ++i) {
        CardInfoPtr info = cardsToSort->at(i);
        if (info) {
            QString currentCriteriaValue = info->getProperty(activeSortCriteria);
            if (currentCriteriaValue != lastSortCriteriaValue) {
                // Save the current tuple (criteriaValue, list) and start a new list
                sortedCardGroups.append(qMakePair(lastSortCriteriaValue, activeList));
                activeList.clear();
                lastSortCriteriaValue = currentCriteriaValue;
            }
            activeList.append(info);
        } else {
            qDebug() << "Card not found in database!";
        }
    }

    // Add the last tuple to sortedCardGroups
    if (!activeList.isEmpty()) {
        sortedCardGroups.append(qMakePair(lastSortCriteriaValue, activeList));
    }

    return sortedCardGroups;
}

/*void VisualDeckEditorWidget::sortCards()
{
    QStringList sortCriteria;
    sortCriteria.append(activeSortCriteria);
    this->sortCardList(sortCriteria, Qt::SortOrder::AscendingOrder);
    QString lastSortCriteriaValue = "";
    overlapCategories = new QWidget(scrollArea);
    overlapCategoriesLayout = new QVBoxLayout(overlapCategories);
    OverlapWidget *printings_group_widget = new OverlapWidget(80, 1, 1, Qt::Vertical, overlapCategories);
    overlap_control_widget->connectOverlapWidget(printings_group_widget);
    for (int i = 0; i < cards->size(); i++) {
        CardInfoPtr info = cards->at(i);
        if (info) {
            if (info->getProperty(activeSortCriteria) != lastSortCriteriaValue) {
                printings_group_widget->adjustMaxColumnsAndRows();
                lastSortCriteriaValue = info->getProperty(activeSortCriteria);
                printings_group_widget = new OverlapWidget(80, 1, 1, Qt::Vertical, overlapCategories);
                overlapCategoriesLayout->addWidget(printings_group_widget);
                overlap_control_widget->connectOverlapWidget(printings_group_widget);
            }
            CardInfoPictureWithTextOverlayWidget* display = new
CardInfoPictureWithTextOverlayWidget(printings_group_widget, true);

            display->setCard(info);
            printings_group_widget->addWidget(display);
            update();
        } else {
            qDebug() << "Card not found in database!";
        }
    }
    printings_group_widget->adjustMaxColumnsAndRows();

    scrollArea->setWidget(overlapCategories);
    update();
}*/

void VisualDeckEditorWidget::onHover(CardInfoPtr hoveredCard)
{
    emit activeCardChanged(hoveredCard);
}

void VisualDeckEditorWidget::onMainboardClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance)
{
    emit mainboardCardClicked(event, instance);
}

void VisualDeckEditorWidget::onSideboardClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance)
{
    emit sideboardCardClicked(event, instance);
}

void VisualDeckEditorWidget::populateCards()
{
    mainDeckCards->clear();
    sideboardCards->clear();

    if (!deckListModel)
        return;

    mainDeckCards = deckListModel->getCardsAsCardInfoPtrsForZone(DECK_ZONE_MAIN);
    sideboardCards = deckListModel->getCardsAsCardInfoPtrsForZone(DECK_ZONE_SIDE);
}

void VisualDeckEditorWidget::sortCardList(const QStringList properties, Qt::SortOrder order = Qt::AscendingOrder)
{
    CardInfoComparator comparator(properties, order);
    std::sort(mainDeckCards->begin(), mainDeckCards->end(), comparator);
    std::sort(sideboardCards->begin(), sideboardCards->end(), comparator);
}
