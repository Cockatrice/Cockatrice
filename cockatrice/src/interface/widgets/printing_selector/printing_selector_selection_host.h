/**
 * @file printing_selector_selection_host.h
 * @ingroup PrintingWidgets
 * @brief A PrintingSelectorHost for hosts that only want the picked printing.
 */

#ifndef PRINTING_SELECTOR_SELECTION_HOST_H
#define PRINTING_SELECTOR_SELECTION_HOST_H

#include "printing_selector_host.h"

/**
 * @brief Host for a PrintingSelector used outside a deck editor.
 *
 * There is no deck: the selector hides its deck-only controls and reports a
 * picked printing through its own printingSelected() signal.
 */
class PrintingSelectorSelectionHost final : public PrintingSelectorHost
{
public:
    [[nodiscard]] DeckStateManager *getDeckStateManager() const override
    {
        return nullptr;
    }

    void showCard(const ExactCard & /*card*/) override
    {
    }

    void showRelatedCard(const QString & /*cardName*/) override
    {
    }

    [[nodiscard]] bool isSelectionOnly() const override
    {
        return true;
    }
};

#endif // PRINTING_SELECTOR_SELECTION_HOST_H
