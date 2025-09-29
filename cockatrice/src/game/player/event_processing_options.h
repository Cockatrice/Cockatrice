/**
 * @file event_processing_options.h
 * @ingroup GameLogicPlayers
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_EVENT_PROCESSING_OPTIONS_H
#define COCKATRICE_EVENT_PROCESSING_OPTIONS_H

#include <QFlags>

// Define the base enum
enum EventProcessingOption
{
    SKIP_REVEAL_WINDOW = 0x0001,
    SKIP_TAP_ANIMATION = 0x0002
};

// Wrap it in a QFlags typedef
Q_DECLARE_FLAGS(EventProcessingOptions, EventProcessingOption)

// Add operator overloads (|, &, etc.)
Q_DECLARE_OPERATORS_FOR_FLAGS(EventProcessingOptions)

#endif // COCKATRICE_EVENT_PROCESSING_OPTIONS_H
