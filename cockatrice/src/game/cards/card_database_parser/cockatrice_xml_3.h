#ifndef COCKATRICE_XML3_H
#define COCKATRICE_XML3_H

#include "card_database_parser.h"

#include <QLoggingCategory>
#include <QXmlStreamReader>

inline Q_LOGGING_CATEGORY(CockatriceXml3Log, "cockatrice_xml.debug.xml_3_parser")

    class CockatriceXml3Parser : public ICardDatabaseParser
{
    Q_OBJECT
    Q_INTERFACES(ICardDatabaseParser)
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