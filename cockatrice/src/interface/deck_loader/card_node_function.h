#ifndef COCKATRICE_DECK_FUNCTION_H
#define COCKATRICE_DECK_FUNCTION_H

class DecklistCardNode;
class InnerDecklistNode;

/**
 * Functions to be used with DeckList::forEachCard
 */
namespace CardNodeFunction
{

/**
 * @brief Sets the providerId of the card to the preferred printing.
 */
struct SetProviderIdToPreferred
{
    void operator()(const InnerDecklistNode *node, DecklistCardNode *card) const;
};

/**
 * @brief Clears all fields on the card related to the printing
 */
struct ClearPrintingData
{
    void operator()(const InnerDecklistNode *node, DecklistCardNode *card) const;
};

/**
 * @brief Sets the providerId of the card based on its set name and collector number.
 */
struct ResolveProviderId
{
    void operator()(const InnerDecklistNode *node, DecklistCardNode *card) const;
};

} // namespace CardNodeFunction

#endif // COCKATRICE_DECK_FUNCTION_H
