#include "tab_deck_storage_visual.h"

#include "../../../game/cards/card_database_model.h"
#include "../../ui/widgets/cards/deck_preview_card_picture_widget.h"
#include "../../ui/widgets/visual_deck_storage/visual_deck_storage_widget.h"
#include "../tab_supervisor.h"
#include "pb/command_deck_del.pb.h"

#include <QMouseEvent>

TabDeckStorageVisual::TabDeckStorageVisual(TabSupervisor *_tabSupervisor)
    : Tab(_tabSupervisor), visualDeckStorageWidget(new VisualDeckStorageWidget(this))
{
    connect(this, &TabDeckStorageVisual::openDeckEditor, tabSupervisor, &TabSupervisor::addDeckEditorTab);
    connect(visualDeckStorageWidget, &VisualDeckStorageWidget::deckPreviewDoubleClicked, this,
            &TabDeckStorageVisual::actOpenLocalDeck);

    auto *widget = new QWidget(this);
    auto *layout = new QVBoxLayout(widget);
    widget->setLayout(layout);
    this->setCentralWidget(widget);
    layout->addWidget(visualDeckStorageWidget);
}

void TabDeckStorageVisual::actOpenLocalDeck(QMouseEvent * /*event*/, DeckPreviewWidget *instance)
{
    DeckLoader deckLoader;
    if (!deckLoader.loadFromFile(instance->filePath, DeckLoader::getFormatFromName(instance->filePath), true)) {
        return;
    }

    emit openDeckEditor(&deckLoader);
}
