#ifndef COCKATRICE_XML4_H
#define COCKATRICE_XML4_H

#include "carddatabaseparser.h"

#include <QXmlStreamReader>

class CockatriceXml4Parser : public ICardDatabaseParser
{
    Q_OBJECT
    Q_INTERFACES(ICardDatabaseParser)
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
signals:
    void addCard(CardInfoPtr card) override;
    void addSet(CardSetPtr set) override;
};

#endif