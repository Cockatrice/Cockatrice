#ifndef COCKATRICE_XML3_H
#define COCKATRICE_XML3_H

#include <QXmlStreamReader>

#include "carddatabaseparser.h"

class CockatriceXml3Parser : public ICardDatabaseParser
{
    Q_OBJECT
    Q_INTERFACES(ICardDatabaseParser)
public:
    CockatriceXml3Parser() = default;
    ~CockatriceXml3Parser() override = default;
    bool getCanParseFile(const QString &name, QIODevice &device) override;
    void parseFile(QIODevice &device) override;
    bool saveToFile(SetNameMap sets, CardNameMap cards, const QString &fileName) override;

private:
    void loadCardsFromXml(QXmlStreamReader &xml);
    void loadSetsFromXml(QXmlStreamReader &xml);
    QString getMainCardType(QString &type);
signals:
    void addCard(CardInfoPtr card) override;
    void addSet(CardSetPtr set) override;
};

#endif