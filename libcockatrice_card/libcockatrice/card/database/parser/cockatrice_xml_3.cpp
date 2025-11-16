#include "cockatrice_xml_3.h"

#include "../../relation/card_relation.h"
#include "../../relation/card_relation_type.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>
#include <version_string.h>

#define COCKATRICE_XML3_TAGNAME "cockatrice_carddatabase"
#define COCKATRICE_XML3_TAGVER 3
#define COCKATRICE_XML3_SCHEMALOCATION                                                                                 \
    "https://raw.githubusercontent.com/Cockatrice/Cockatrice/master/doc/carddatabase_v3/cards.xsd"

bool CockatriceXml3Parser::getCanParseFile(const QString &fileName, QIODevice &device)
{
    qCInfo(CockatriceXml3Log) << "Trying to parse: " << fileName;

    if (!fileName.endsWith(".xml", Qt::CaseInsensitive)) {
        qCInfo(CockatriceXml3Log) << "Parsing failed: wrong extension";
        return false;
    }

    QXmlStreamReader xml(&device);
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::StartElement) {
            if (xml.name().toString() == COCKATRICE_XML3_TAGNAME) {
                int version = xml.attributes().value("version").toString().toInt();
                if (version == COCKATRICE_XML3_TAGVER) {
                    return true;
                } else {
                    qCInfo(CockatriceXml3Log) << "Parsing failed: wrong version" << version;
                    return false;
                }

            } else {
                qCInfo(CockatriceXml3Log) << "Parsing failed: wrong element tag" << xml.name();
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

                auto name = xml.name().toString();
                if (name == "sets") {
                    loadSetsFromXml(xml);
                } else if (name == "cards") {
                    loadCardsFromXml(xml);
                } else if (!name.isEmpty()) {
                    qCInfo(CockatriceXml3Log) << "Unknown item" << name << ", trying to continue anyway";
                    xml.skipCurrentElement();
                }
            }
        }
    }

    if (xml.hasError()) {
        QString preamble = tr("Parse error at line %1 col %2:").arg(xml.lineNumber()).arg(xml.columnNumber());
        qCWarning(CockatriceXml3Log).noquote() << preamble << xml.errorString();
    }
}

void CockatriceXml3Parser::loadSetsFromXml(QXmlStreamReader &xml)
{
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::EndElement) {
            break;
        }

        auto name = xml.name().toString();
        if (name == "set") {
            QString shortName, longName, setType;
            QDate releaseDate;
            while (!xml.atEnd()) {
                if (xml.readNext() == QXmlStreamReader::EndElement) {
                    break;
                }
                name = xml.name().toString();

                if (name == "name") {
                    shortName = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                } else if (name == "longname") {
                    longName = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                } else if (name == "settype") {
                    setType = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                } else if (name == "releasedate") {
                    releaseDate =
                        QDate::fromString(xml.readElementText(QXmlStreamReader::IncludeChildElements), Qt::ISODate);
                } else if (!name.isEmpty()) {
                    qCInfo(CockatriceXml3Log) << "Unknown set property" << name << ", trying to continue anyway";
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

        auto xmlName = xml.name().toString();
        if (xmlName == "card") {
            QString name = QString("");
            QString text = QString("");
            QVariantHash properties = QVariantHash();
            QString colors = QString("");
            QList<CardRelation *> relatedCards, reverseRelatedCards;
            auto _sets = SetToPrintingsMap();
            int tableRow = 0;
            bool cipt = false;
            bool landscapeOrientation = false;
            bool isToken = false;
            bool upsideDown = false;

            while (!xml.atEnd()) {
                if (xml.readNext() == QXmlStreamReader::EndElement) {
                    break;
                }
                xmlName = xml.name().toString();

                // variable - assigned properties
                if (xmlName == "name") {
                    name = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                } else if (xmlName == "text") {
                    text = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                } else if (xmlName == "color" || xmlName == "colors") {
                    colors.append(xml.readElementText(QXmlStreamReader::IncludeChildElements));
                } else if (xmlName == "token") {
                    isToken = static_cast<bool>(xml.readElementText(QXmlStreamReader::IncludeChildElements).toInt());
                    // generic properties
                } else if (xmlName == "manacost") {
                    properties.insert("manacost", xml.readElementText(QXmlStreamReader::IncludeChildElements));
                } else if (xmlName == "cmc") {
                    properties.insert("cmc", xml.readElementText(QXmlStreamReader::IncludeChildElements));
                } else if (xmlName == "type") {
                    QString type = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                    properties.insert("type", type);
                    properties.insert("maintype", getMainCardType(type));
                } else if (xmlName == "pt") {
                    properties.insert("pt", xml.readElementText(QXmlStreamReader::IncludeChildElements));
                } else if (xmlName == "loyalty") {
                    properties.insert("loyalty", xml.readElementText(QXmlStreamReader::IncludeChildElements));
                    // positioning info
                } else if (xmlName == "tablerow") {
                    tableRow = xml.readElementText(QXmlStreamReader::IncludeChildElements).toInt();
                } else if (xmlName == "cipt") {
                    cipt = (xml.readElementText(QXmlStreamReader::IncludeChildElements) == "1");
                } else if (xmlName == "landscapeOrientation") {
                    landscapeOrientation = (xml.readElementText(QXmlStreamReader::IncludeChildElements) == "1");
                } else if (xmlName == "upsidedown") {
                    upsideDown = (xml.readElementText(QXmlStreamReader::IncludeChildElements) == "1");
                    // sets
                } else if (xmlName == "set") {
                    // NOTE: attributes must be read before readElementText()
                    QXmlStreamAttributes attrs = xml.attributes();
                    QString setName = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                    PrintingInfo setInfo(internalAddSet(setName));
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
                    _sets[setName].append(setInfo);
                    // related cards
                } else if (xmlName == "related" || xmlName == "reverse-related") {
                    CardRelationType attach = CardRelationType::DoesNotAttach;
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
                        attach = CardRelationType::AttachTo;
                    }

                    if (attrs.hasAttribute("exclude")) {
                        exclude = true;
                    }

                    auto *relation = new CardRelation(cardName, attach, exclude, variable, count);
                    if (xmlName == "reverse-related") {
                        reverseRelatedCards << relation;
                    } else {
                        relatedCards << relation;
                    }
                } else if (!xmlName.isEmpty()) {
                    qCInfo(CockatriceXml3Log) << "Unknown card property" << xmlName << ", trying to continue anyway";
                    xml.skipCurrentElement();
                }
            }

            if (name.isEmpty()) {
                qCWarning(CockatriceXml3Log) << "Encountered card with empty name; skipping";
                continue;
            }

            properties.insert("colors", colors);

            CardInfo::UiAttributes attributes = {.cipt = cipt,
                                                 .landscapeOrientation = landscapeOrientation,
                                                 .tableRow = tableRow,
                                                 .upsideDownArt = upsideDown};
            CardInfoPtr newCard = CardInfo::newInstance(name, text, isToken, properties, relatedCards,
                                                        reverseRelatedCards, _sets, attributes);
            emit addCard(newCard);
        }
    }
}

static QXmlStreamWriter &operator<<(QXmlStreamWriter &xml, const CardSetPtr &set)
{
    if (set.isNull()) {
        qCWarning(CockatriceXml3Log) << "&operator<< set is nullptr";
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
        qCWarning(CockatriceXml3Log) << "operator<< info is nullptr";
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
    const SetToPrintingsMap setMap = info->getSets();
    for (const auto &printings : setMap) {
        for (const PrintingInfo &set : printings) {
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

            xml.writeCharacters(set.getSet()->getShortName());
            xml.writeEndElement();
        }
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
    const CardInfo::UiAttributes &attributes = info->getUiAttributes();
    xml.writeTextElement("tablerow", QString::number(attributes.tableRow));
    if (attributes.cipt) {
        xml.writeTextElement("cipt", "1");
    }
    if (attributes.landscapeOrientation) {
        xml.writeTextElement("landscapeOrientation", "1");
    }
    if (attributes.upsideDownArt) {
        xml.writeTextElement("upsidedown", "1");
    }

    xml.writeEndElement(); // card

    return xml;
}

bool CockatriceXml3Parser::saveToFile(SetNameMap _sets,
                                      CardNameMap cards,
                                      const QString &fileName,
                                      const QString &sourceUrl,
                                      const QString &sourceVersion)
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
    xml.writeAttribute("xmlns:xsi", COCKATRICE_XML_XSI_NAMESPACE);
    xml.writeAttribute("xsi:schemaLocation", COCKATRICE_XML3_SCHEMALOCATION);

    xml.writeStartElement("info");
    xml.writeTextElement("author", QCoreApplication::applicationName() + QString(" %1").arg(VERSION_STRING));
    xml.writeTextElement("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    xml.writeTextElement("sourceUrl", sourceUrl);
    xml.writeTextElement("sourceVersion", sourceVersion);
    xml.writeEndElement();

    if (_sets.count() > 0) {
        xml.writeStartElement("sets");
        for (CardSetPtr set : _sets) {
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
