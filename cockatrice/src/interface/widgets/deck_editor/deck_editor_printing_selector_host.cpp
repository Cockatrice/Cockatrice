#include "deck_editor_printing_selector_host.h"

#include "../tabs/abstract_tab_deck_editor.h"

#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/card/database/card_database_querier.h>
#include <libcockatrice/utility/card_ref.h>

DeckEditorPrintingSelectorHost::DeckEditorPrintingSelectorHost(AbstractTabDeckEditor *_deckEditor)
    : deckEditor(_deckEditor)
{
}

DeckStateManager *DeckEditorPrintingSelectorHost::getDeckStateManager() const
{
    return deckEditor->deckStateManager;
}

void DeckEditorPrintingSelectorHost::showCard(const ExactCard &card)
{
    deckEditor->updateCard(card);
}

void DeckEditorPrintingSelectorHost::showRelatedCard(const QString &cardName)
{
    deckEditor->updateCard(CardDatabaseManager::query()->getCard(CardRef{.name = cardName}));
    deckEditor->showPrintingSelector();
}

bool DeckEditorPrintingSelectorHost::isSelectionOnly() const
{
    return false;
}
