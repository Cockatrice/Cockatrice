#include "deck_edit_event_bus.h"

DeckEditEventBus::DeckEditEventBus()
{
}

/**
 * Gets the singleton instance of the event bus
 */
DeckEditEventBus *DeckEditEventBus::instance()
{
    static DeckEditEventBus deckEditEventBus; // Created only once, on first access
    return &deckEditEventBus;
}