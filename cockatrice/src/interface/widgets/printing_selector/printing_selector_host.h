/**
 * @file printing_selector_host.h
 * @ingroup PrintingWidgets
 * @brief Host services a PrintingSelector needs beyond its own widgets.
 */

#ifndef PRINTING_SELECTOR_HOST_H
#define PRINTING_SELECTOR_HOST_H

#include <QString>

class DeckStateManager;
class ExactCard;

/**
 * @brief Dependency boundary between PrintingSelector and the surface hosting it.
 *
 * The deck editor supplies deck data and routes card activations back to its
 * info pane. The token dialog uses the selection-only host, which has no deck.
 */
class PrintingSelectorHost
{
public:
    virtual ~PrintingSelectorHost() = default;

    /** @brief Deck state used for amounts and in-deck ordering, or nullptr when there is no deck. */
    [[nodiscard]] virtual DeckStateManager *getDeckStateManager() const = 0;

    /** @brief Shows a printing on the host's card info surface. */
    virtual void showCard(const ExactCard &card) = 0;

    /** @brief Opens a related card through the host. */
    virtual void showRelatedCard(const QString &cardName) = 0;

    /** @brief True when picking a printing only reports it instead of changing a deck. */
    [[nodiscard]] virtual bool isSelectionOnly() const = 0;
};

#endif // PRINTING_SELECTOR_HOST_H
