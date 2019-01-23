#ifndef CARDDATABASE_PARSER_H
#define CARDDATABASE_PARSER_H

#include <QIODevice>
#include <QString>

#include "../carddatabase.h"

class ICardDatabaseParser : public QObject
{
public:
    ~ICardDatabaseParser() override = default;

    virtual bool getCanParseFile(const QString &name, QIODevice &device) = 0;
    virtual void parseFile(QIODevice &device) = 0;
    virtual bool saveToFile(SetNameMap sets, CardNameMap cards, const QString &fileName) = 0;
    static void clearSetlist();

protected:
    /*
     * A cached list of the available sets, needed to cross-reference sets from cards.
     * Shared between all parsers
     */
    static SetNameMap sets;

    CardSetPtr internalAddSet(const QString &setName,
                              const QString &longName = "",
                              const QString &setType = "",
                              const QDate &releaseDate = QDate());
signals:
    virtual void addCard(CardInfoPtr card) = 0;
    virtual void addSet(CardSetPtr set) = 0;
};

Q_DECLARE_INTERFACE(ICardDatabaseParser, "ICardDatabaseParser")

#endif