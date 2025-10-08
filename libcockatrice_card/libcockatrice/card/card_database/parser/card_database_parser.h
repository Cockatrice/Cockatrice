/**
 * @file card_database_parser.h
 * @ingroup CardDatabaseParsers
 * @brief The ICardDatabaseParser defines the base interface for parser sub-classes.
 */

#ifndef CARDDATABASE_PARSER_H
#define CARDDATABASE_PARSER_H

#include "../../card_info.h"

#include <QIODevice>
#include <QString>

#define COCKATRICE_XML_XSI_NAMESPACE "http://www.w3.org/2001/XMLSchema-instance"

class ICardDatabaseParser : public QObject
{
    Q_OBJECT
public:
    ~ICardDatabaseParser() override = default;

    virtual bool getCanParseFile(const QString &name, QIODevice &device) = 0;
    virtual void parseFile(QIODevice &device) = 0;
    virtual bool saveToFile(SetNameMap sets,
                            CardNameMap cards,
                            const QString &fileName,
                            const QString &sourceUrl = "unknown",
                            const QString &sourceVersion = "unknown") = 0;
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
                              const QDate &releaseDate = QDate(),
                              const CardSet::Priority priority = CardSet::PriorityFallback);
signals:
    void addCard(CardInfoPtr card);
    void addSet(CardSetPtr set);
};

Q_DECLARE_INTERFACE(ICardDatabaseParser, "ICardDatabaseParser")

#endif
