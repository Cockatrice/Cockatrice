#ifndef CARDDATABASE_PARSER_H
#define CARDDATABASE_PARSER_H

#include <QString>
#include <QIODevice>

#include "../carddatabase.h"

class ICardDatabaseParser : public QObject
{
public:
    virtual ~ICardDatabaseParser() {}
    virtual bool getCanParseFile(const QString &name, QIODevice &device) = 0;
    virtual void parseFile(QIODevice &device) = 0;
    virtual bool saveToFile(SetNameMap sets, CardNameMap cards, const QString &fileName) = 0;
signals:
    virtual void addCard(CardInfoPtr card) = 0;
};

Q_DECLARE_INTERFACE(ICardDatabaseParser, "ICardDatabaseParser")

#endif