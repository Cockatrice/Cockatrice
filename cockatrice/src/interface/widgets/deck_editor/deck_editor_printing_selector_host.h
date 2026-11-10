/**
 * @file deck_editor_printing_selector_host.h
 * @ingroup DeckEditorWidgets
 * @brief Connects a PrintingSelector to a deck editor tab.
 */

#ifndef DECK_EDITOR_PRINTING_SELECTOR_HOST_H
#define DECK_EDITOR_PRINTING_SELECTOR_HOST_H

#include "../printing_selector/printing_selector_host.h"

class AbstractTabDeckEditor;

/** @brief Deck-editor implementation of PrintingSelectorHost. */
class DeckEditorPrintingSelectorHost final : public PrintingSelectorHost
{
public:
    explicit DeckEditorPrintingSelectorHost(AbstractTabDeckEditor *deckEditor);

    [[nodiscard]] DeckStateManager *getDeckStateManager() const override;
    void showCard(const ExactCard &card) override;
    void showRelatedCard(const QString &cardName) override;
    [[nodiscard]] bool isSelectionOnly() const override;

private:
    AbstractTabDeckEditor *deckEditor;
};

#endif // DECK_EDITOR_PRINTING_SELECTOR_HOST_H
