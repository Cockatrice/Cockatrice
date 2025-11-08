#include "cockatrice_xml_4.h"

#include "../../relation/card_relation.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>
#include <version_string.h>

#define COCKATRICE_XML4_TAGNAME "cockatrice_carddatabase"
#define COCKATRICE_XML4_TAGVER 4
#define COCKATRICE_XML4_SCHEMALOCATION                                                                                 \
    "https://raw.githubusercontent.com/Cockatrice/Cockatrice/master/doc/carddatabase_v4/cards.xsd"

CockatriceXml4Parser::CockatriceXml4Parser(ICardPreferenceProvider *_cardPreferenceProvider)
    : cardPreferenceProvider(_cardPreferenceProvider)
{
}

bool CockatriceXml4Parser::getCanParseFile(const QString &fileName, QIODevice &device)
{
    qCInfo(CockatriceXml4Log) << "Trying to parse: " << fileName;

    if (!fileName.endsWith(".xml", Qt::CaseInsensitive)) {
        qCInfo(CockatriceXml4Log) << "Parsing failed: wrong extension";
        return false;
    }

    QXmlStreamReader xml(&device);
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::StartElement) {
            if (xml.name().toString() == COCKATRICE_XML4_TAGNAME) {
                int version = xml.attributes().value("version").toString().toInt();
                if (version == COCKATRICE_XML4_TAGVER) {
                    return true;
                } else {
                    qCInfo(CockatriceXml4Log) << "Parsing failed: wrong version" << version;
                    return false;
                }

            } else {
                qCInfo(CockatriceXml4Log) << "Parsing failed: wrong element tag" << xml.name();
                return false;
            }
        }
    }

    return true;
}

void CockatriceXml4Parser::parseFile(QIODevice &device)
{
    QXmlStreamReader xml(&device);
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::StartElement) {
            while (!xml.atEnd()) {
                if (xml.readNext() == QXmlStreamReader::EndElement) {
                    break;
                }

                auto xmlName = xml.name().toString();
                if (xmlName == "sets") {
                    loadSetsFromXml(xml);
                } else if (xmlName == "cards") {
                    loadCardsFromXml(xml);
                } else if (!xmlName.isEmpty()) {
                    qCInfo(CockatriceXml4Log) << "Unknown item" << xmlName << ", trying to continue anyway";
                    xml.skipCurrentElement();
                }
            }
        }
    }

    if (xml.hasError()) {
        QString preamble = tr("Parse error at line %1 col %2:").arg(xml.lineNumber()).arg(xml.columnNumber());
        qCWarning(CockatriceXml4Log).noquote() << preamble << xml.errorString();
    }
}

void CockatriceXml4Parser::loadSetsFromXml(QXmlStreamReader &xml)
{
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::EndElement) {
            break;
        }

        auto xmlName = xml.name().toString();
        if (xmlName == "set") {
            QString shortName, longName, setType;
            QDate releaseDate;
            short priority;
            while (!xml.atEnd()) {
                if (xml.readNext() == QXmlStreamReader::EndElement) {
                    break;
                }
                xmlName = xml.name().toString();

                if (xmlName == "name") {
                    shortName = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                } else if (xmlName == "longname") {
                    longName = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                } else if (xmlName == "settype") {
                    setType = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                } else if (xmlName == "releasedate") {
                    releaseDate =
                        QDate::fromString(xml.readElementText(QXmlStreamReader::IncludeChildElements), Qt::ISODate);
                } else if (xmlName == "priority") {
                    priority = xml.readElementText(QXmlStreamReader::IncludeChildElements).toShort();
                } else if (!xmlName.isEmpty()) {
                    qCInfo(CockatriceXml4Log) << "Unknown set property" << xmlName << ", trying to continue anyway";
                    xml.skipCurrentElement();
                }
            }

            internalAddSet(shortName, longName, setType, releaseDate, static_cast<CardSet::Priority>(priority));
        }
    }
}

QVariantHash CockatriceXml4Parser::loadCardPropertiesFromXml(QXmlStreamReader &xml)
{
    QVariantHash properties = QVariantHash();
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::EndElement) {
            break;
        }

        auto xmlName = xml.name().toString();
        if (!xmlName.isEmpty()) {
            properties.insert(xmlName, xml.readElementText(QXmlStreamReader::IncludeChildElements));
        }
    }
    return properties;
}

void CockatriceXml4Parser::loadCardsFromXml(QXmlStreamReader &xml)
{
    bool includeRebalancedCards = cardPreferenceProvider->getIncludeRebalancedCards();
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::EndElement) {
            break;
        }

        auto xmlName = xml.name().toString();

        if (xmlName == "card") {
            QString name = QString("");
            QString text = QString("");
            QVariantHash properties = QVariantHash();
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
                } else if (xmlName == "token") {
                    isToken = static_cast<bool>(xml.readElementText(QXmlStreamReader::IncludeChildElements).toInt());
                    // generic properties
                } else if (xmlName == "prop") {
                    properties = loadCardPropertiesFromXml(xml);
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
                    // NOTE: attributes but be read before readElementText()
                    QXmlStreamAttributes attrs = xml.attributes();
                    QString setName = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                    auto set = internalAddSet(setName);
                    if (set->getEnabled()) {
                        PrintingInfo printingInfo(set);
                        for (QXmlStreamAttribute attr : attrs) {
                            QString attrName = attr.name().toString();
                            if (attrName == "picURL")
                                attrName = "picurl";
                            printingInfo.setProperty(attrName, attr.value().toString());
                        }

                        // This is very much a hack and not the right place to
                        // put this check, as it requires a reload of Cockatrice
                        // to be apply.
                        //
                        // However, this is also true of the `set->getEnabled()`
                        // check above (which is currently bugged as well), so
                        // we'll fix both at the same time.
                        if (includeRebalancedCards || printingInfo.getProperty("isRebalanced") != "true") {
                            _sets[setName].append(printingInfo);
                        }
                    }
                    // related cards
                } else if (xmlName == "related" || xmlName == "reverse-related") {
                    CardRelationType attachType = CardRelationType::DoesNotAttach;
                    bool exclude = false;
                    bool variable = false;
                    bool persistent = false;
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
                        attachType = attrs.value("attach").toString() == "transform" ? CardRelationType::TransformInto
                                                                                     : CardRelationType::AttachTo;
                    }

                    if (attrs.hasAttribute("exclude")) {
                        exclude = true;
                    }

                    if (attrs.hasAttribute("persistent")) {
                        persistent = true;
                    }

                    auto *relation = new CardRelation(cardName, attachType, exclude, variable, count, persistent);
                    if (xmlName == "reverse-related") {
                        reverseRelatedCards << relation;
                    } else {
                        relatedCards << relation;
                    }
                } else if (!xmlName.isEmpty()) {
                    qCInfo(CockatriceXml4Log) << "Unknown card property" << xmlName << ", trying to continue anyway";
                    xml.skipCurrentElement();
                }
            }

            if (name.isEmpty()) {
                qCWarning(CockatriceXml4Log) << "Encountered card with empty name; skipping";
                continue;
            }

            CardInfoPtr newCard =
                CardInfo::newInstance(name, text, isToken, properties, relatedCards, reverseRelatedCards, _sets, cipt,
                                      landscapeOrientation, tableRow, upsideDown);
            emit addCard(newCard);
        }
    }
}

static QXmlStreamWriter &operator<<(QXmlStreamWriter &xml, const CardSetPtr &set)
{
    if (set.isNull()) {
        qCWarning(CockatriceXml4Log) << "&operator<< set is nullptr";
        return xml;
    }

    xml.writeStartElement("set");
    xml.writeTextElement("name", set->getShortName());
    xml.writeTextElement("longname", set->getLongName());
    xml.writeTextElement("settype", set->getSetType());
    xml.writeTextElement("releasedate", set->getReleaseDate().toString(Qt::ISODate));
    xml.writeTextElement("priority", QString::number(set->getPriority()));
    xml.writeEndElement();

    return xml;
}

static QXmlStreamWriter &operator<<(QXmlStreamWriter &xml, const CardInfoPtr &info)
{
    if (info.isNull()) {
        qCWarning(CockatriceXml4Log) << "operator<< info is nullptr";
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
    xml.writeStartElement("prop");
    for (QString propName : info->getProperties()) {
        xml.writeTextElement(propName, info->getProperty(propName));
    }
    xml.writeEndElement();

    // sets
    for (const auto &printings : info->getSets()) {
        for (const PrintingInfo &set : printings) {
            xml.writeStartElement("set");
            for (const QString &propName : set.getProperties()) {
                xml.writeAttribute(propName, set.getProperty(propName));
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
            xml.writeAttribute("attach", i->getAttachTypeAsString());
        }
        if (i->getIsCreateAllExclusion()) {
            xml.writeAttribute("exclude", "exclude");
        }
        if (i->getIsPersistent()) {
            xml.writeAttribute("persistent", "persistent");
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
            xml.writeAttribute("attach", i->getAttachTypeAsString());
        }

        if (i->getIsCreateAllExclusion()) {
            xml.writeAttribute("exclude", "exclude");
        }

        if (i->getIsPersistent()) {
            xml.writeAttribute("persistent", "persistent");
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
    if (info->getLandscapeOrientation()) {
        xml.writeTextElement("landscapeOrientation", "1");
    }
    if (info->getUpsideDownArt()) {
        xml.writeTextElement("upsidedown", "1");
    }

    xml.writeEndElement(); // card

    return xml;
}

bool CockatriceXml4Parser::saveToFile(SetNameMap _sets,
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
    xml.writeStartElement(COCKATRICE_XML4_TAGNAME);
    xml.writeAttribute("version", QString::number(COCKATRICE_XML4_TAGVER));
    xml.writeAttribute("xmlns:xsi", COCKATRICE_XML_XSI_NAMESPACE);
    xml.writeAttribute("xsi:schemaLocation", COCKATRICE_XML4_SCHEMALOCATION);

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
