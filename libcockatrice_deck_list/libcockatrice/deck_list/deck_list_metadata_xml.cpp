#include "deck_list.h"

#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>
#include <libcockatrice/utility/playmat_params.h>

namespace
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
double parseClampedParam(const QString &valueString, double fallback, double min, double max)
{
    bool ok = false;
    const double value = valueString.toDouble(&ok);
    if (!ok) {
        return fallback;
    }
    return qBound(min, value, max);
}

} // namespace

bool DeckList::Metadata::readElement(QXmlStreamReader *xml, const QString &childName)
{
    if (childName == "lastLoadedTimestamp") {
        lastLoadedTimestamp = xml->readElementText();
    } else if (childName == "deckname") {
        name = xml->readElementText();
    } else if (childName == "format") {
        gameFormat = xml->readElementText();
    } else if (childName == "comments") {
        comments = xml->readElementText();
    } else if (childName == "bannerCard") {
        QString providerId = xml->attributes().value("providerId").toString();
        QString cardName = xml->readElementText();
        bannerCard = {cardName, providerId};
    } else if (childName == "playmatCard") {
        QString providerId = xml->attributes().value("providerId").toString();
        // Attributes are read before readElementText consumes the element.
        QString marginLStr = xml->attributes().value("marginPctL").toString();
        QString marginRStr = xml->attributes().value("marginPctR").toString();
        QString vOffStr = xml->attributes().value("verticalOffset").toString();
        QString zoomStr = xml->attributes().value("zoom").toString();
        QString cardName = xml->readElementText();
        playmat.card = {cardName, providerId};
        // Clamp to the same ranges as the settings dialog and the remote
        // player-properties path so malformed deck files cannot produce
        // degenerate art rectangles (e.g. a zoom of 0 dividing by zero).
        playmat.params.marginPctL = parseClampedParam(marginLStr, 0.07, 0.0, 0.95);
        playmat.params.marginPctR = parseClampedParam(marginRStr, 0.07, 0.0, 0.95);
        playmat.params.verticalOffset = parseClampedParam(vOffStr, 0.33, 0.0, 1.0);
        playmat.params.zoom = parseClampedParam(zoomStr, 1.0, 0.1, 4.0);
    } else if (childName == "tags") {
        tags.clear(); // Clear existing tags
        while (xml->readNextStartElement()) {
            if (xml->name().toString() == "tag") {
                tags.append(xml->readElementText());
            }
        }
    } else {
        return false;
    }
    return true;
}

void DeckList::Metadata::write(QXmlStreamWriter *xml) const
{
    xml->writeTextElement("lastLoadedTimestamp", lastLoadedTimestamp);
    xml->writeTextElement("deckname", name);
    xml->writeTextElement("format", gameFormat);
    xml->writeStartElement("bannerCard");
    xml->writeAttribute("providerId", bannerCard.providerId);
    xml->writeCharacters(bannerCard.name);
    xml->writeEndElement();
    if (!playmat.card.isEmpty()) {
        xml->writeStartElement("playmatCard");
        xml->writeAttribute("providerId", playmat.card.providerId);
        xml->writeAttribute("marginPctL", QString::number(playmat.params.marginPctL, 'f', 4));
        xml->writeAttribute("marginPctR", QString::number(playmat.params.marginPctR, 'f', 4));
        xml->writeAttribute("verticalOffset", QString::number(playmat.params.verticalOffset, 'f', 4));
        xml->writeAttribute("zoom", QString::number(playmat.params.zoom, 'f', 4));
        xml->writeCharacters(playmat.card.name);
        xml->writeEndElement();
    }
    xml->writeTextElement("comments", comments);

    // Write tags
    xml->writeStartElement("tags");
    for (const QString &tag : tags) {
        xml->writeTextElement("tag", tag);
    }
    xml->writeEndElement();
}
