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
                    shortName = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                } else if (xml.name() == "longname") {
                    longName = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                } else if (xml.name() == "settype") {
                    setType = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                } else if (xml.name() == "releasedate") {
                    releaseDate =
                        QDate::fromString(xml.readElementText(QXmlStreamReader::IncludeChildElements), Qt::ISODate);
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

QString CockatriceXml3Parser::getMainCardType(QString &type)
{
    QString result = type;
    /*
    Legendary Artifact Creature - Golem
    Instant // Instant
    */

    int pos;
    if ((pos = result.indexOf('-')) != -1) {
        result.remove(pos, result.length());
    }

    if ((pos = result.indexOf("—")) != -1) {
        result.remove(pos, result.length());
    }

    if ((pos = result.indexOf("//")) != -1) {
        result.remove(pos, result.length());
    }

    result = result.simplified();
    /*
    Legendary Artifact Creature
    Instant
    */

    if ((pos = result.lastIndexOf(' ')) != -1) {
        result = result.mid(pos + 1);
    }
    /*
    Creature
    Instant
    */

    return result;
}

void CockatriceXml3Parser::loadCardsFromXml(QXmlStreamReader &xml)
{
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::EndElement) {
            break;
        }

        if (xml.name() == "card") {
            QString name = QString("");
            QString text = QString("");
            QVariantHash properties = QVariantHash();
            QString colors = QString("");
            QList<CardRelation *> relatedCards, reverseRelatedCards;
            CardInfoPerSetMap sets = CardInfoPerSetMap();
            int tableRow = 0;
            bool cipt = false;
            bool isToken = false;
            bool upsideDown = false;

            while (!xml.atEnd()) {
                if (xml.readNext() == QXmlStreamReader::EndElement) {
                    break;
                }
                // variable - assigned properties
                if (xml.name() == "name") {
                    name = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                } else if (xml.name() == "text") {
                    text = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                } else if (xml.name() == "color") {
                    colors.append(xml.readElementText(QXmlStreamReader::IncludeChildElements));
                } else if (xml.name() == "token") {
                    isToken = static_cast<bool>(xml.readElementText(QXmlStreamReader::IncludeChildElements).toInt());
                    // generic properties
                } else if (xml.name() == "manacost") {
                    properties.insert("manacost", xml.readElementText(QXmlStreamReader::IncludeChildElements));
                } else if (xml.name() == "cmc") {
                    properties.insert("cmc", xml.readElementText(QXmlStreamReader::IncludeChildElements));
                } else if (xml.name() == "type") {
                    QString type = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                    properties.insert("type", type);
                    properties.insert("maintype", getMainCardType(type));
                } else if (xml.name() == "pt") {
                    properties.insert("pt", xml.readElementText(QXmlStreamReader::IncludeChildElements));
                } else if (xml.name() == "loyalty") {
                    properties.insert("loyalty", xml.readElementText(QXmlStreamReader::IncludeChildElements));
                    // positioning info
                } else if (xml.name() == "tablerow") {
                    tableRow = xml.readElementText(QXmlStreamReader::IncludeChildElements).toInt();
                } else if (xml.name() == "cipt") {
                    cipt = (xml.readElementText(QXmlStreamReader::IncludeChildElements) == "1");
                } else if (xml.name() == "upsidedown") {
                    upsideDown = (xml.readElementText(QXmlStreamReader::IncludeChildElements) == "1");
                    // sets
                } else if (xml.name() == "set") {
                    // NOTE: attributes must be read before readElementText()
                    QXmlStreamAttributes attrs = xml.attributes();
                    QString setName = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                    CardInfoPerSet setInfo(internalAddSet(setName));
                    if (attrs.hasAttribute("muId")) {
                        setInfo.setProperty("muid", attrs.value("muId").toString());
                    }

                    if (attrs.hasAttribute("muId")) {
                        setInfo.setProperty("uuid", attrs.value("uuId").toString());
                    }

                    if (attrs.hasAttribute("picURL")) {
                        setInfo.setProperty("picurl", attrs.value("picURL").toString());
                    }

                    if (attrs.hasAttribute("num")) {
                        setInfo.setProperty("num", attrs.value("num").toString());
                    }

                    if (attrs.hasAttribute("rarity")) {
                        setInfo.setProperty("rarity", attrs.value("rarity").toString());
                    }
                    sets.insert(setName, setInfo);
                    // relatd cards
                } else if (xml.name() == "related" || xml.name() == "reverse-related") {
                    bool attach = false;
                    bool exclude = false;
                    bool variable = false;
                    int count = 1;
                    QXmlStreamAttributes attrs = xml.attributes();
                    QString cardName = xml.readElementText(QXmlStreamReader::IncludeChildElements);
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
                } else if (xml.name() != "") {
                    qDebug() << "[CockatriceXml3Parser] Unknown card property" << xml.name()
                             << ", trying to continue anyway";
                    xml.skipCurrentElement();
                }
            }

            properties.insert("colors", colors);
            CardInfoPtr newCard = CardInfo::newInstance(name, text, isToken, properties, relatedCards,
                                                        reverseRelatedCards, sets, cipt, tableRow, upsideDown);
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

    QString tmpString;

    xml.writeStartElement("card");

    // variable - assigned properties
    xml.writeTextElement("name", info->getName());
    xml.writeTextElement("text", info->getText());
    if (info->getIsToken()) {
        xml.writeTextElement("token", "1");
    }

    // generic properties
    xml.writeTextElement("manacost", info->getProperty("manacost"));
    xml.writeTextElement("cmc", info->getProperty("cmc"));
    xml.writeTextElement("type", info->getProperty("type"));

    int colorSize = info->getColors().size();
    for (int i = 0; i < colorSize; ++i) {
        xml.writeTextElement("color", info->getColors().at(i));
    }

    tmpString = info->getProperty("pt");
    if (!tmpString.isEmpty()) {
        xml.writeTextElement("pt", tmpString);
    }

    tmpString = info->getProperty("loyalty");
    if (!tmpString.isEmpty()) {
        xml.writeTextElement("loyalty", tmpString);
    }

    // sets
    const CardInfoPerSetMap sets = info->getSets();
    for (CardInfoPerSet set : sets) {
        xml.writeStartElement("set");
        xml.writeAttribute("rarity", set.getProperty("rarity"));
        xml.writeAttribute("muId", set.getProperty("muid"));
        xml.writeAttribute("uuId", set.getProperty("uuid"));

        tmpString = set.getProperty("num");
        if (!tmpString.isEmpty()) {
            xml.writeAttribute("num", tmpString);
        }

        tmpString = set.getProperty("picurl");
        if (!tmpString.isEmpty()) {
            xml.writeAttribute("picURL", tmpString);
        }

        xml.writeCharacters(set.getPtr()->getShortName());
        xml.writeEndElement();
    }

    // related cards
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

    // positioning
    xml.writeTextElement("tablerow", QString::number(info->getTableRow()));
    if (info->getCipt()) {
        xml.writeTextElement("cipt", "1");
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