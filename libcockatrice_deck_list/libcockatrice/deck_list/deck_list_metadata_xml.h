#ifndef COCKATRICE_DECK_LIST_METADATA_XML_H
#define COCKATRICE_DECK_LIST_METADATA_XML_H

#include "deck_list.h"

#include <QString>

class QXmlStreamReader;
class QXmlStreamWriter;

namespace DeckListMetadataXml
{

/**
 * @brief Reads a single deck metadata element from a Cockatrice deck XML stream.
 *
 * @param xml Reader positioned at the element.
 * @param childName Name of the current element.
 * @param metadata Metadata to fill in.
 * @return true if a metadata element was consumed, false if @p childName is
 *         not a metadata element.
 */
bool readElement(QXmlStreamReader *xml, const QString &childName, DeckList::Metadata &metadata);

/**
 * @brief Writes the deck metadata section of a Cockatrice deck XML file.
 *
 * @param xml Writer to append the metadata elements to.
 * @param metadata The metadata to serialize.
 */
void write(QXmlStreamWriter *xml, const DeckList::Metadata &metadata);

} // namespace DeckListMetadataXml

#endif // COCKATRICE_DECK_LIST_METADATA_XML_H