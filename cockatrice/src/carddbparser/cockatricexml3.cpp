#include "cockatricexml3.h"

#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>

#define COCKATRICE_XML3_TAGNAME "cockatrice_carddatabase"
#define COCKATRICE_XML3_TAGVER 3

bool CockatriceXml3Parser::getCanParseFile(const QString &fileName, QIODevice &device)
{
    qDebug() << "[CockatriceXml3Parser] Trying to parse: " << fileName;

    if (!fileName.endsWith(".xml", Qt::CaseInsensitive)) {
        qDebug() << "[CockatriceXml3Parser] Parsing failed: wrong extension";
        return false;
    }

    QXmlStreamReader xml(&device);
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::StartElement) {
            if (xml.name() == COCKATRICE_XML3_TAGNAME) {
                int version = xml.attributes().value("version").toString().toInt();
                if (version == COCKATRICE_XML3_TAGVER) {
                    return true;
                } else {
                    qDebug() << "[CockatriceXml3Parser] Parsing failed: wrong version" << version;
                    return false;
                }

            } else {
                qDebug() << "[CockatriceXml3Parser] Parsing failed: wrong element tag" << xml.name();
                return false;
            }
        }
    }

    return true;
}

void CockatriceXml3Parser::parseFile(QIODevice &device)
{
    QXmlStreamReader xml(&device);
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::StartElement) {
            while (!xml.atEnd()) {
                if (xml.readNext() == QXmlStreamReader::EndElement) {
                    break;
                }

                if (xml.name() == "sets") {
                    loadSetsFromXml(xml);
                } else if (xml.name() == "cards") {
                    loadCardsFromXml(xml);
                } else if (xml.name() != "") {
                    qDebug() << "[CockatriceXml3Parser] Unknown item" << xml.name() << ", trying to continue anyway";
                    xml.skipCurrentElement();
                }
            }
        }
    }
}

CardSetPtr CockatriceXml3Parser::internalAddSet(const QString &setName,
                                                const QString &longName,
                                                const QString &setType,
                                                const QDate &releaseDate)
{
    if (sets.contains(setName)) {
        return sets.value(setName);
    }

    CardSetPtr newSet = CardSet::newInstance(setName);
    newSet->setLongName(longName);
    newSet->setSetType(setType);
    newSet->setReleaseDate(releaseDate);

    sets.insert(setName, newSet);
    emit addSet(newSet);
    return newSet;
}

void CockatriceXml3Parser::clearSetlist()
{
    sets.clear();
}

void CockatriceXml3Parser::loadSetsFromXml(QXmlStreamReader &xml)
{
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::EndElement) {
            break;
        }

        if (xml.name() == "set") {
            QString shortName, longName, setType;
            QDate releaseDate;
            while (!xml.atEnd()) {
                if (xml.readNext() == QXmlStreamReader::EndElement) {
                    break;
                }

                if (xml.name() == "name") {
                    shortName = xml.readElementText();
                } else if (xml.name() == "longname") {
                    longName = xml.readElementText();
                } else if (xml.name() == "settype") {
                    setType = xml.readElementText();
                } else if (xml.name() == "releasedate") {
                    releaseDate = QDate::fromString(xml.readElementText(), Qt::ISODate);
                } else if (xml.name() != "") {
                    qDebug() << "[CockatriceXml3Parser] Unknown set property" << xml.name()
                             << ", trying to continue anyway";
                    xml.skipCurrentElement();
                }
            }

            internalAddSet(shortName, longName, setType, releaseDate);
        }
    }
}

void CockatriceXml3Parser::loadCardsFromXml(QXmlStreamReader &xml)
{
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::EndElement) {
            break;
        }

        if (xml.name() == "card") {
            QString name, manacost, cmc, type, pt, text, loyalty;
            QStringList colors;
            QList<CardRelation *> relatedCards, reverseRelatedCards;
            QStringMap customPicURLs;
            MuidMap muids;
            QStringMap uuids, collectorNumbers, rarities;
            SetList sets;
            int tableRow = 0;
            bool cipt = false;
            bool isToken = false;
            bool upsideDown = false;
            while (!xml.atEnd()) {
                if (xml.readNext() == QXmlStreamReader::EndElement) {
                    break;
                }

                if (xml.name() == "name") {
                    name = xml.readElementText();
                } else if (xml.name() == "manacost") {
                    manacost = xml.readElementText();
                } else if (xml.name() == "cmc") {
                    cmc = xml.readElementText();
                } else if (xml.name() == "type") {
                    type = xml.readElementText();
                } else if (xml.name() == "pt") {
                    pt = xml.readElementText();
                } else if (xml.name() == "text") {
                    text = xml.readElementText();
                } else if (xml.name() == "set") {
                    QXmlStreamAttributes attrs = xml.attributes();
                    QString setName = xml.readElementText();
                    sets.append(internalAddSet(setName));
                    if (attrs.hasAttribute("muId")) {
                        muids[setName] = attrs.value("muId").toString().toInt();
                    }

                    if (attrs.hasAttribute("muId")) {
                        uuids[setName] = attrs.value("uuId").toString();
                    }

                    if (attrs.hasAttribute("picURL")) {
                        customPicURLs[setName] = attrs.value("picURL").toString();
                    }

                    if (attrs.hasAttribute("num")) {
                        collectorNumbers[setName] = attrs.value("num").toString();
                    }

                    if (attrs.hasAttribute("rarity")) {
                        rarities[setName] = attrs.value("rarity").toString();
                    }
                } else if (xml.name() == "color") {
                    colors << xml.readElementText();
                } else if (xml.name() == "related" || xml.name() == "reverse-related") {
                    bool attach = false;
                    bool exclude = false;
                    bool variable = false;
                    int count = 1;
                    QXmlStreamAttributes attrs = xml.attributes();
                    QString cardName = xml.readElementText();
                    if (attrs.hasAttribute("count")) {
                        if (attrs.value("count").toString().indexOf("x=") == 0) {
                            variable = true;
                            count = attrs.value("count").toString().remove(0, 2).toInt();
                        } else if (attrs.value("count").toString().indexOf("x") == 0) {
                            variable = true;
                        } else {
                            count = attrs.value("count").toString().toInt();
                        }

                        if (count < 1) {
                            count = 1;
                        }
                    }

                    if (attrs.hasAttribute("attach")) {
                        attach = true;
                    }

                    if (attrs.hasAttribute("exclude")) {
                        exclude = true;
                    }

                    auto *relation = new CardRelation(cardName, attach, exclude, variable, count);
                    if (xml.name() == "reverse-related") {
                        reverseRelatedCards << relation;
                    } else {
                        relatedCards << relation;
                    }
                } else if (xml.name() == "tablerow") {
                    tableRow = xml.readElementText().toInt();
                } else if (xml.name() == "cipt") {
                    cipt = (xml.readElementText() == "1");
                } else if (xml.name() == "upsidedown") {
                    upsideDown = (xml.readElementText() == "1");
                } else if (xml.name() == "loyalty") {
                    loyalty = xml.readElementText();
                } else if (xml.name() == "token") {
                    isToken = static_cast<bool>(xml.readElementText().toInt());
                } else if (xml.name() != "") {
                    qDebug() << "[CockatriceXml3Parser] Unknown card property" << xml.name()
                             << ", trying to continue anyway";
                    xml.skipCurrentElement();
                }
            }

            CardInfoPtr newCard = CardInfo::newInstance(
                name, isToken, manacost, cmc, type, pt, text, colors, relatedCards, reverseRelatedCards, upsideDown,
                loyalty, cipt, tableRow, sets, customPicURLs, muids, uuids, collectorNumbers, rarities);
            emit addCard(newCard);
        }
    }
}

static QXmlStreamWriter &operator<<(QXmlStreamWriter &xml, const CardSetPtr &set)
{
    if (set.isNull()) {
        qDebug() << "&operator<< set is nullptr";
        return xml;
    }

    xml.writeStartElement("set");
    xml.writeTextElement("name", set->getShortName());
    xml.writeTextElement("longname", set->getLongName());
    xml.writeTextElement("settype", set->getSetType());
    xml.writeTextElement("releasedate", set->getReleaseDate().toString(Qt::ISODate));
    xml.writeEndElement();

    return xml;
}

static QXmlStreamWriter &operator<<(QXmlStreamWriter &xml, const CardInfoPtr &info)
{
    if (info.isNull()) {
        qDebug() << "operator<< info is nullptr";
        return xml;
    }

    xml.writeStartElement("card");
    xml.writeTextElement("name", info->getName());

    const SetList &sets = info->getSets();
    QString tmpString;
    QString tmpSet;
    for (int i = 0; i < sets.size(); i++) {
        xml.writeStartElement("set");

        tmpSet = sets[i]->getShortName();
        xml.writeAttribute("rarity", info->getRarity(tmpSet));
        xml.writeAttribute("muId", QString::number(info->getMuId(tmpSet)));
        xml.writeAttribute("uuId", info->getUuId(tmpSet));

        tmpString = info->getCollectorNumber(tmpSet);
        if (!tmpString.isEmpty()) {
            xml.writeAttribute("num", info->getCollectorNumber(tmpSet));
        }

        tmpString = info->getCustomPicURL(tmpSet);
        if (!tmpString.isEmpty()) {
            xml.writeAttribute("picURL", tmpString);
        }

        xml.writeCharacters(tmpSet);
        xml.writeEndElement();
    }
    const QStringList &colors = info->getColors();
    for (int i = 0; i < colors.size(); i++) {
        xml.writeTextElement("color", colors[i]);
    }

    const QList<CardRelation *> related = info->getRelatedCards();
    for (auto i : related) {
        xml.writeStartElement("related");
        if (i->getDoesAttach()) {
            xml.writeAttribute("attach", "attach");
        }
        if (i->getIsCreateAllExclusion()) {
            xml.writeAttribute("exclude", "exclude");
        }

        if (i->getIsVariable()) {
            if (1 == i->getDefaultCount()) {
                xml.writeAttribute("count", "x");
            } else {
                xml.writeAttribute("count", "x=" + QString::number(i->getDefaultCount()));
            }
        } else if (1 != i->getDefaultCount()) {
            xml.writeAttribute("count", QString::number(i->getDefaultCount()));
        }
        xml.writeCharacters(i->getName());
        xml.writeEndElement();
    }
    const QList<CardRelation *> reverseRelated = info->getReverseRelatedCards();
    for (auto i : reverseRelated) {
        xml.writeStartElement("reverse-related");
        if (i->getDoesAttach()) {
            xml.writeAttribute("attach", "attach");
        }

        if (i->getIsCreateAllExclusion()) {
            xml.writeAttribute("exclude", "exclude");
        }

        if (i->getIsVariable()) {
            if (1 == i->getDefaultCount()) {
                xml.writeAttribute("count", "x");
            } else {
                xml.writeAttribute("count", "x=" + QString::number(i->getDefaultCount()));
            }
        } else if (1 != i->getDefaultCount()) {
            xml.writeAttribute("count", QString::number(i->getDefaultCount()));
        }
        xml.writeCharacters(i->getName());
        xml.writeEndElement();
    }
    xml.writeTextElement("manacost", info->getManaCost());
    xml.writeTextElement("cmc", info->getCmc());
    xml.writeTextElement("type", info->getCardType());
    if (!info->getPowTough().isEmpty()) {
        xml.writeTextElement("pt", info->getPowTough());
    }
    xml.writeTextElement("tablerow", QString::number(info->getTableRow()));
    xml.writeTextElement("text", info->getText());
    if (info->getMainCardType() == "Planeswalker") {
        xml.writeTextElement("loyalty", info->getLoyalty());
    }
    if (info->getCipt()) {
        xml.writeTextElement("cipt", "1");
    }
    if (info->getIsToken()) {
        xml.writeTextElement("token", "1");
    }
    if (info->getUpsideDownArt()) {
        xml.writeTextElement("upsidedown", "1");
    }

    xml.writeEndElement(); // card

    return xml;
}

bool CockatriceXml3Parser::saveToFile(SetNameMap sets, CardNameMap cards, const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QXmlStreamWriter xml(&file);

    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement(COCKATRICE_XML3_TAGNAME);
    xml.writeAttribute("version", QString::number(COCKATRICE_XML3_TAGVER));

    if (sets.count() > 0) {
        xml.writeStartElement("sets");
        for (CardSetPtr set : sets) {
            xml << set;
        }
        xml.writeEndElement();
    }

    if (cards.count() > 0) {
        xml.writeStartElement("cards");
        for (CardInfoPtr card : cards) {
            xml << card;
        }
        xml.writeEndElement();
    }

    xml.writeEndElement(); // cockatrice_carddatabase
    xml.writeEndDocument();

    return true;
}