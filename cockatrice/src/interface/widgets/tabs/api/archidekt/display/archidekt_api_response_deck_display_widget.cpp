#include "archidekt_api_response_deck_display_widget.h"

#include "../../../../../deck_loader/deck_loader.h"
#include "../../../../cards/card_info_picture_with_text_overlay_widget.h"
#include "../../../../cards/card_size_widget.h"
#include "../../../../cards/deck_card_zone_display_widget.h"
#include "../api_response/deck/archidekt_api_response_deck.h"

#include <QSortFilterProxyModel>
#include <libcockatrice/card/database/card_database_manager.h>

ArchidektApiResponseDeckDisplayWidget::ArchidektApiResponseDeckDisplayWidget(QWidget *parent,
                                                                             ArchidektApiResponseDeck _response,
                                                                             CardSizeWidget *_cardSizeSlider)
    : QWidget(parent), response(_response), cardSizeSlider(_cardSizeSlider)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    openInEditorButton = new QPushButton(this);
    layout->addWidget(openInEditorButton);

    connect(openInEditorButton, &QPushButton::clicked, this,
            &ArchidektApiResponseDeckDisplayWidget::actOpenInDeckEditor);

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    layout->addWidget(scrollArea);

    container = new QWidget(scrollArea);

    scrollArea->setWidget(container);

    containerLayout = new QVBoxLayout(container);
    container->setLayout(containerLayout);

    zoneContainer = new QWidget(container);
    containerLayout->addWidget(zoneContainer);
    zoneContainerLayout = new QVBoxLayout(zoneContainer);
    zoneContainer->setLayout(zoneContainerLayout);

    QString tempDeck;
    QTextStream deckStream(&tempDeck);

    for (auto card : response.getCards()) {
        tempDeck += QString("%1 %2 (%3) %4\n")
                        .arg(card.getQuantity())
                        .arg(card.getCard().getOracleCard().value("name").toString())
                        .arg(card.getCard().getEdition().getEditionCode())
                        .arg(card.getCard().getCollectorNumber());
    }

    model = new DeckListModel(this);
    connect(model, &DeckListModel::modelReset, this, &ArchidektApiResponseDeckDisplayWidget::decklistModelReset);
    model->getDeckList()->loadFromStream_Plain(deckStream, false);

    DeckLoader::resolveSetNameAndNumberToProviderID(model->getDeckList());

    model->rebuildTree();

    retranslateUi();
}

void ArchidektApiResponseDeckDisplayWidget::retranslateUi()
{
    openInEditorButton->setText(tr("Open Deck in Deck Editor"));
}

void ArchidektApiResponseDeckDisplayWidget::actOpenInDeckEditor()
{
    auto loader = new DeckLoader(this);
    loader->getDeckList()->loadFromString_Native(model->getDeckList()->writeToString_Native());

    loader->getDeckList()->setName(response.getDeckName());

    emit openInDeckEditor(loader);
}

void ArchidektApiResponseDeckDisplayWidget::clearAllDisplayWidgets()
{
    for (auto idx : indexToWidgetMap.keys()) {
        auto displayWidget = indexToWidgetMap.value(idx);
        zoneContainerLayout->removeWidget(displayWidget);
        indexToWidgetMap.remove(idx);
        delete displayWidget;
    }
}

void ArchidektApiResponseDeckDisplayWidget::decklistModelReset()
{
    clearAllDisplayWidgets();
    constructZoneWidgetsFromDeckListModel();
}

void ArchidektApiResponseDeckDisplayWidget::constructZoneWidgetsFromDeckListModel()
{
    qDebug() << model->rowCount(model->getRoot());
    QSortFilterProxyModel proxy;
    proxy.setSourceModel(model);
    proxy.setSortRole(Qt::EditRole);
    proxy.sort(1, Qt::AscendingOrder);

    for (int i = 0; i < proxy.rowCount(); ++i) {
        QModelIndex proxyIndex = proxy.index(i, 0);
        QModelIndex sourceIndex = proxy.mapToSource(proxyIndex);

        // Make a persistent index from the *source* model
        QPersistentModelIndex persistent(sourceIndex);

        if (indexToWidgetMap.contains(persistent)) {
            continue;
        }

        DeckCardZoneDisplayWidget *zoneDisplayWidget =
            new DeckCardZoneDisplayWidget(zoneContainer, model, nullptr, persistent,
                                          model->data(persistent.sibling(persistent.row(), 1), Qt::EditRole).toString(),
                                          "maintype", {"name"}, DisplayType::Overlap, 20, 10, cardSizeSlider);
        /*
        connect(this, &VisualDeckEditorWidget::activeSortCriteriaChanged, zoneDisplayWidget,
                &DeckCardZoneDisplayWidget::onActiveSortCriteriaChanged);
        connect(this, &VisualDeckEditorWidget::activeGroupCriteriaChanged, zoneDisplayWidget,
                &DeckCardZoneDisplayWidget::onActiveGroupCriteriaChanged);
        connect(this, &VisualDeckEditorWidget::displayTypeChanged, zoneDisplayWidget,
                &DeckCardZoneDisplayWidget::refreshDisplayType);*/
        zoneContainerLayout->addWidget(zoneDisplayWidget);

        indexToWidgetMap.insert(persistent, zoneDisplayWidget);
    }
}

void ArchidektApiResponseDeckDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    layout->invalidate();
    layout->activate();
    layout->update();
}
