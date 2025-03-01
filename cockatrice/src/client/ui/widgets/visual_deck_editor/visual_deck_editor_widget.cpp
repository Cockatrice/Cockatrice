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
    connect(deckListModel, &DeckListModel::dataChanged, this, &VisualDeckEditorWidget::decklistDataChanged);

    // The Main Widget and Main Layout, which contain a single Widget: The Scroll Area
    this->setMinimumSize(0, 0);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);

    groupByComboBox = new QComboBox();
    QStringList groupProperties;
    groupProperties << "maintype"
                    << "colors"
                    << "cmc"
                    << "name";
    groupByComboBox->addItems(groupProperties);
    connect(groupByComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this,
            &VisualDeckEditorWidget::actChangeActiveGroupCriteria);
    actChangeActiveGroupCriteria();

    sortByComboBox = new QComboBox();
    QStringList sortProperties;
    sortProperties << "name"
                   << "cmc"
                   << "colors"
                   << "maintype";
    sortByComboBox->addItems(sortProperties);
    connect(sortByComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this,
            &VisualDeckEditorWidget::actChangeActiveSortCriteria);
    actChangeActiveSortCriteria();

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

    updateZoneWidgets();

    overlapControlWidget = new OverlapControlWidget(80, 1, 1, Qt::Vertical, this);

    mainLayout->addWidget(groupByComboBox);
    mainLayout->addWidget(sortByComboBox);
    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(overlapControlWidget);
}

void VisualDeckEditorWidget::updateZoneWidgets()
{
    addZoneIfDoesNotExist();
    deleteZoneIfDoesNotExist();
}

void VisualDeckEditorWidget::addZoneIfDoesNotExist()
{
    QList<DeckCardZoneDisplayWidget *> cardZoneDisplayWidgets =
        zoneContainer->findChildren<DeckCardZoneDisplayWidget *>();
    for (const QString &zone : *deckListModel->getZones()) {
        bool found = false;
        for (DeckCardZoneDisplayWidget *displayWidget : cardZoneDisplayWidgets) {
            if (displayWidget->zoneName == zone) {
                found = true;
            }
        }

        if (!found) {
            DeckCardZoneDisplayWidget *zoneDisplayWidget = new DeckCardZoneDisplayWidget(
                zoneContainer, deckListModel, zone, activeGroupCriteria, activeSortCriteria, 20, 10);
            connect(zoneDisplayWidget, &DeckCardZoneDisplayWidget::cardHovered, this, &VisualDeckEditorWidget::onHover);
            connect(zoneDisplayWidget, &DeckCardZoneDisplayWidget::cardClicked, this,
                    &VisualDeckEditorWidget::onCardClick);
            connect(this, &VisualDeckEditorWidget::activeSortCriteriaChanged, zoneDisplayWidget,
                    &DeckCardZoneDisplayWidget::onActiveSortCriteriaChanged);
            connect(this, &VisualDeckEditorWidget::activeGroupCriteriaChanged, zoneDisplayWidget,
                    &DeckCardZoneDisplayWidget::onActiveGroupCriteriaChanged);
            zoneContainerLayout->addWidget(zoneDisplayWidget);
        }
    }
}

void VisualDeckEditorWidget::deleteZoneIfDoesNotExist()
{
    QList<DeckCardZoneDisplayWidget *> cardZoneDisplayWidgets =
        zoneContainer->findChildren<DeckCardZoneDisplayWidget *>();
    for (DeckCardZoneDisplayWidget *displayWidget : cardZoneDisplayWidgets) {
        bool found = false;
        for (const QString &zone : *deckListModel->getZones()) {

            if (displayWidget->zoneName == zone) {
                found = true;
            }
        }

        if (!found) {
            zoneContainerLayout->removeWidget(displayWidget);
        }
    }
}

void VisualDeckEditorWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    zoneContainer->setMaximumWidth(scrollArea->viewport()->width());
}

void VisualDeckEditorWidget::actChangeActiveGroupCriteria()
{
    activeGroupCriteria = groupByComboBox->currentText();
    qDebug() << "VDE says we group by " << activeGroupCriteria;
    emit activeGroupCriteriaChanged(activeGroupCriteria);
}

void VisualDeckEditorWidget::actChangeActiveSortCriteria()
{
    activeSortCriteria = sortByComboBox->currentText();
    qDebug() << "VDE says we sort by " << activeSortCriteria;
    emit activeSortCriteriaChanged(activeSortCriteria);
}

void VisualDeckEditorWidget::decklistDataChanged(QModelIndex topLeft, QModelIndex bottomRight)
{
    // Might use these at some point.
    (void)topLeft;
    (void)bottomRight;
    // Necessary to delay this in this manner else the updateDisplay will nuke widgets while their onClick event
    // hasn't returned yet. Interval of 0 means QT will schedule this after the current event loop has finished.
    updateZoneWidgets();
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

void VisualDeckEditorWidget::onCardClick(QMouseEvent *event,
                                         CardInfoPictureWithTextOverlayWidget *instance,
                                         QString zoneName)
{
    emit cardClicked(event, instance, zoneName);
}
