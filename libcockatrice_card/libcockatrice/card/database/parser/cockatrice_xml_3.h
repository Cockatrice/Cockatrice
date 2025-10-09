/**
 * @file cockatrice_xml_3.h
 * @ingroup CardDatabaseParsers
 * @brief The CockatriceXml3Parser is capable of parsing version 3 of the Cockatrice XML Schema.
 */

#ifndef COCKATRICE_XML3_H
#define COCKATRICE_XML3_H

#include "card_database_parser.h"

#include <QLoggingCategory>
#include <QXmlStreamReader>

inline Q_LOGGING_CATEGORY(CockatriceXml3Log, "cockatrice_xml.xml_3_parser");

class CockatriceXml3Parser : public ICardDatabaseParser
{
    Q_OBJECT
public:
    CockatriceXml3Parser() = default;
    ~CockatriceXml3Parser() override = default;
    bool getCanParseFile(const QString &name, QIODevice &device) override;
    void parseFile(QIODevice &device) override;
    bool saveToFile(SetNameMap _sets,
                    CardNameMap cards,
                    const QString &fileName,
                    const QString &sourceUrl = "unknown",
                    const QString &sourceVersion = "unknown") override;

private:
    void loadCardsFromXml(QXmlStreamReader &xml);
    void loadSetsFromXml(QXmlStreamReader &xml);
    QString getMainCardType(QString &type);
};

#endif