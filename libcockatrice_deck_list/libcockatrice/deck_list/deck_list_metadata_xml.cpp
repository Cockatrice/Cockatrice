#include "deck_list_metadata_xml.h"

#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>
#include <libcockatrice/utility/playmat_params.h>

namespace DeckListMetadataXml
{

/**
 * @brief Parses a floating point XML attribute into a clamped playmat parameter.
 *
 * Falls back to @p fallback when the attribute is missing or malformed, so
 * malformed deck files cannot produce degenerate art rectangles (e.g. a zoom
 * of 0 dividing by zero).
 *
 * @param valueString Raw attribute text.
 * @param fallback Value used when the text cannot be parsed.
 * @param min Lower clamp bound.
 * @param max Upper clamp bound.
 * @return The parsed value clamped to [min, max], or @p fallback.
 */
static double parseClampedParam(const QString &valueString, double fallback, double min, double max)
{
    bool ok = false;
    const double value = valueString.toDouble(&ok);
    if (!ok) {
        return fallback;
    }
    return qBound(min, value, max);
}

bool readElement(QXmlStreamReader *xml, const QString &childName, DeckList::Metadata &metadata)
{
    if (childName == "lastLoadedTimestamp") {
        metadata.lastLoadedTimestamp = xml->readElementText();
    } else if (childName == "deckname") {
        metadata.name = xml->readElementText();
    } else if (childName == "format") {
        metadata.gameFormat = xml->readElementText();
    } else if (childName == "comments") {
        metadata.comments = xml->readElementText();
    } else if (childName == "bannerCard") {
        QString providerId = xml->attributes().value("providerId").toString();
        QString cardName = xml->readElementText();
        metadata.bannerCard = {cardName, providerId};
    } else if (childName == "playmatCard") {
        QString providerId = xml->attributes().value("providerId").toString();
        // Attributes are read before readElementText consumes the element.
        QString marginLStr = xml->attributes().value("marginPctL").toString();
        QString marginRStr = xml->attributes().value("marginPctR").toString();
        QString vOffStr = xml->attributes().value("verticalOffset").toString();
        QString zoomStr = xml->attributes().value("zoom").toString();
        QString cardName = xml->readElementText();
        PlaymatInfo playmat;
        playmat.card = {cardName, providerId};
        // Clamp to the same ranges as the settings dialog and the remote
        // player-properties path so malformed deck files cannot produce
        // degenerate art rectangles (e.g. a zoom of 0 dividing by zero).
        playmat.params.marginPctL = parseClampedParam(marginLStr, 0.07, 0.0, 0.95);
        playmat.params.marginPctR = parseClampedParam(marginRStr, 0.07, 0.0, 0.95);
        playmat.params.verticalOffset = parseClampedParam(vOffStr, 0.33, 0.0, 1.0);
        playmat.params.zoom = parseClampedParam(zoomStr, 1.0, 0.1, 4.0);
        metadata.playmat = playmat;
    } else if (childName == "tags") {
        metadata.tags.clear(); // Clear existing tags
        while (xml->readNextStartElement()) {
            if (xml->name().toString() == "tag") {
                metadata.tags.append(xml->readElementText());
            }
        }
    } else {
        return false;
    }
    return true;
}

void write(QXmlStreamWriter *xml, const DeckList::Metadata &metadata)
{
    xml->writeTextElement("lastLoadedTimestamp", metadata.lastLoadedTimestamp);
    xml->writeTextElement("deckname", metadata.name);
    xml->writeTextElement("format", metadata.gameFormat);
    xml->writeStartElement("bannerCard");
    xml->writeAttribute("providerId", metadata.bannerCard.providerId);
    xml->writeCharacters(metadata.bannerCard.name);
    xml->writeEndElement();
    if (!metadata.playmat.card.isEmpty()) {
        xml->writeStartElement("playmatCard");
        xml->writeAttribute("providerId", metadata.playmat.card.providerId);
        xml->writeAttribute("marginPctL", QString::number(metadata.playmat.params.marginPctL, 'f', 4));
        xml->writeAttribute("marginPctR", QString::number(metadata.playmat.params.marginPctR, 'f', 4));
        xml->writeAttribute("verticalOffset", QString::number(metadata.playmat.params.verticalOffset, 'f', 4));
        xml->writeAttribute("zoom", QString::number(metadata.playmat.params.zoom, 'f', 4));
        xml->writeCharacters(metadata.playmat.card.name);
        xml->writeEndElement();
    }
    xml->writeTextElement("comments", metadata.comments);

    // Write tags
    xml->writeStartElement("tags");
    for (const QString &tag : metadata.tags) {
        xml->writeTextElement("tag", tag);
    }
    xml->writeEndElement();
}

} // namespace DeckListMetadataXml