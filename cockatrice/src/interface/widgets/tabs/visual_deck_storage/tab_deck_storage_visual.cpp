#include "tab_deck_storage_visual.h"

#include "../../interface/widgets/cards/deck_preview_card_picture_widget.h"
#include "../../interface/widgets/visual_deck_storage/visual_deck_storage_widget.h"
#include "../tab_supervisor.h"

#include <QMessageBox>
#include <QMouseEvent>
#include <libcockatrice/models/database/card_database_model.h>
#include <libcockatrice/protocol/pb/command_deck_del.pb.h>

TabDeckStorageVisual::TabDeckStorageVisual(TabSupervisor *_tabSupervisor)
    : Tab(_tabSupervisor), visualDeckStorageWidget(new VisualDeckStorageWidget(this))
{
    connect(this, &TabDeckStorageVisual::openDeckEditor, tabSupervisor, &TabSupervisor::openDeckInNewTab);
    connect(visualDeckStorageWidget, &VisualDeckStorageWidget::deckLoadRequested, this,
            &TabDeckStorageVisual::actOpenLocalDeck);
    connect(visualDeckStorageWidget, &VisualDeckStorageWidget::openDeckEditor, this,
            &TabDeckStorageVisual::openDeckEditor);

    auto *widget = new QWidget(this);
    auto *layout = new QVBoxLayout(widget);
    widget->setLayout(layout);
    this->setCentralWidget(widget);
    layout->addWidget(visualDeckStorageWidget);
}

void TabDeckStorageVisual::actOpenLocalDeck(const QString &filePath)
{
    DeckLoader deckLoader(this);
    if (!deckLoader.loadFromFile(filePath, DeckLoader::getFormatFromName(filePath), true)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open deck at %1").arg(filePath));
        return;
    }

    emit openDeckEditor(&deckLoader);
}
