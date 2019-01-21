#ifndef COCKATRICE_XML4_H
#define COCKATRICE_XML4_H

#include <QXmlStreamReader>

#include "carddatabaseparser.h"

class CockatriceXml4Parser : public ICardDatabaseParser
{
    Q_OBJECT
    Q_INTERFACES(ICardDatabaseParser)
public:
    CockatriceXml4Parser() = default;
    ~CockatriceXml4Parser() = default;
    bool getCanParseFile(const QString &name, QIODevice &device);
    void parseFile(QIODevice &device);
    bool saveToFile(SetNameMap sets, CardNameMap cards, const QString &fileName);

private:
    QVariantHash loadCardPropertiesFromXml(QXmlStreamReader &xml);
    void loadCardsFromXml(QXmlStreamReader &xml);
    void loadSetsFromXml(QXmlStreamReader &xml);
signals:
    void addCard(CardInfoPtr card);
    void addSet(CardSetPtr set);
};

#endif