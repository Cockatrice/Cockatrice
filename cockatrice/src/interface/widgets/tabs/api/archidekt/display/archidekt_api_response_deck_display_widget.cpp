#include "archidekt_api_response_deck_display_widget.h"

#include "../../../../../deck_loader/card_node_function.h"
#include "../../../../../deck_loader/deck_loader.h"
#include "../../../../cards/card_info_picture_with_text_overlay_widget.h"
#include "../../../../cards/card_size_widget.h"
#include "../../../../cards/deck_card_zone_display_widget.h"
#include "../../../../visual_deck_editor/visual_deck_display_options_widget.h"
#include "../api_response/archidekt_formats.h"
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

    displayOptionsWidget = new VisualDeckDisplayOptionsWidget(this);
    layout->addWidget(displayOptionsWidget);

    connect(displayOptionsWidget, &VisualDeckDisplayOptionsWidget::groupCriteriaChanged, this,
            &ArchidektApiResponseDeckDisplayWidget::onGroupCriteriaChange);

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
        QString fullName = card.getCard().getOracleCard().value("name").toString();
        // We don't really care about the second card, the card database already has it as a relation
        QString cleanName = fullName.split("//").first().trimmed();

        tempDeck += QString("%1 %2 (%3) %4\n")
                        .arg(card.getQuantity())
                        .arg(cleanName)
                        .arg(card.getCard().getEdition().getEditionCode().toUpper())
                        .arg(card.getCard().getCollectorNumber());
    }

    model = new DeckListModel(this);
    connect(model, &DeckListModel::modelReset, this, &ArchidektApiResponseDeckDisplayWidget::decklistModelReset);
    model->getDeckList()->loadFromStream_Plain(deckStream, false);

    model->forEachCard(CardNodeFunction::ResolveProviderId());

    retranslateUi();
}

void ArchidektApiResponseDeckDisplayWidget::retranslateUi()
{
    openInEditorButton->setText(tr("Open Deck in Deck Editor"));
}

void ArchidektApiResponseDeckDisplayWidget::onGroupCriteriaChange(const QString &activeGroupCriteria)
{
    model->setActiveGroupCriteria(DeckListModelGroupCriteria::fromString(activeGroupCriteria));
    model->sort(DeckListModelColumns::CARD_NAME, Qt::AscendingOrder);
}

void ArchidektApiResponseDeckDisplayWidget::actOpenInDeckEditor()
{
    DeckList deckList(*model->getDeckList());
    deckList.setName(response.getDeckName());
    deckList.setGameFormat(
        ArchidektFormats::formatToCockatriceName(ArchidektFormats::DeckFormat(response.getDeckFormat() - 1)));

    LoadedDeck loadedDeck = {deckList, {}};

    emit openInDeckEditor(loadedDeck);
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
    proxy.sort(DeckListModelColumns::CARD_NAME, Qt::AscendingOrder);

    for (int i = 0; i < proxy.rowCount(); ++i) {
        QModelIndex proxyIndex = proxy.index(i, 0);
        QModelIndex sourceIndex = proxy.mapToSource(proxyIndex);

        // Make a persistent index from the *source* model
        QPersistentModelIndex persistent(sourceIndex);

        if (indexToWidgetMap.contains(persistent)) {
            continue;
        }

        QString zoneName =
            persistent.sibling(persistent.row(), DeckListModelColumns::CARD_NAME).data(Qt::EditRole).toString();

        DeckCardZoneDisplayWidget *zoneDisplayWidget =
            new DeckCardZoneDisplayWidget(zoneContainer, model, nullptr, persistent, zoneName, "maintype", {"name"},
                                          DisplayType::Overlap, 20, 10, cardSizeSlider);

        connect(displayOptionsWidget, &VisualDeckDisplayOptionsWidget::sortCriteriaChanged, zoneDisplayWidget,
                &DeckCardZoneDisplayWidget::onActiveSortCriteriaChanged);
        connect(displayOptionsWidget, &VisualDeckDisplayOptionsWidget::groupCriteriaChanged, zoneDisplayWidget,
                &DeckCardZoneDisplayWidget::onActiveGroupCriteriaChanged);
        connect(displayOptionsWidget, &VisualDeckDisplayOptionsWidget::displayTypeChanged, zoneDisplayWidget,
                &DeckCardZoneDisplayWidget::refreshDisplayType);
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
