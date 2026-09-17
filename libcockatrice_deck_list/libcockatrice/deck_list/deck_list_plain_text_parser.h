#ifndef COCKATRICE_DECK_LIST_PLAIN_TEXT_PARSER_H
#define COCKATRICE_DECK_LIST_PLAIN_TEXT_PARSER_H

#include "deck_list.h"

#include <QString>
#include <functional>

class QTextStream;

namespace DeckListPlainText
{

/**
 * @brief Parses a plain-text deck list into a tree and its metadata.
 *
 * Clears the tree first, and clears the metadata unless @p preserveMetadata is
 * true, then fills both from the text.
 *
 * @param in The text to load
 * @param preserveMetadata If true, don't clear the existing metadata
 * @param cardNameNormalizer Function that takes the parsed card name string
 *        in the text and returns the name to store
 * @param metadata Deck metadata written by the parser
 * @param tree Deck tree the parser adds cards to
 * @return False if the input was empty, true otherwise.
 */
bool parse(QTextStream &in,
           bool preserveMetadata,
           const std::function<QString(const QString &)> &cardNameNormalizer,
           DeckList::Metadata &metadata,
           DecklistNodeTree &tree);

} // namespace DeckListPlainText

#endif // COCKATRICE_DECK_LIST_PLAIN_TEXT_PARSER_H