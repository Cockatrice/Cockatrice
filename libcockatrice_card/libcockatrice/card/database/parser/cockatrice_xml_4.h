/**
 * @file cockatrice_xml_4.h
 * @ingroup CardDatabaseParsers
 * @brief The CockatriceXml4Parser is capable of parsing version 4 of the Cockatrice XML Schema.
 */

#ifndef COCKATRICE_XML4_H
#define COCKATRICE_XML4_H

#include "card_database_parser.h"

#include <QLoggingCategory>
#include <QXmlStreamReader>

inline Q_LOGGING_CATEGORY(CockatriceXml4Log, "cockatrice_xml.xml_4_parser");

class CockatriceXml4Parser : public ICardDatabaseParser
{
    Q_OBJECT
public:
    CockatriceXml4Parser() = default;
    ~CockatriceXml4Parser() override = default;
    bool getCanParseFile(const QString &name, QIODevice &device) override;
    void parseFile(QIODevice &device) override;
    bool saveToFile(SetNameMap _sets,
                    CardNameMap cards,
                    const QString &fileName,
                    const QString &sourceUrl = "unknown",
                    const QString &sourceVersion = "unknown") override;

private:
    QVariantHash loadCardPropertiesFromXml(QXmlStreamReader &xml);
    void loadCardsFromXml(QXmlStreamReader &xml);
    void loadSetsFromXml(QXmlStreamReader &xml);
};

#endif