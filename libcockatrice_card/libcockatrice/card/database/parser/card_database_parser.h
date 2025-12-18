#ifndef CARDDATABASE_PARSER_H
#define CARDDATABASE_PARSER_H

#include "../../card_info.h"

#include <QIODevice>
#include <QString>

#define COCKATRICE_XML_XSI_NAMESPACE "http://www.w3.org/2001/XMLSchema-instance"

/**
 * @class ICardDatabaseParser
 * @ingroup CardDatabase
 * @brief Defines the base parser interface (ICardDatabaseParser) for all card database parsers.
 *
 * Provides methods for checking file compatibility, parsing, and saving card databases.
 * Also provides shared access to the global set list for cross-referencing.
 */
class ICardDatabaseParser : public QObject
{
    Q_OBJECT
public:
    ICardDatabaseParser(ICardSetPriorityController *cardSetPriorityController);
    ~ICardDatabaseParser() override = default;

    /**
     * @brief Checks whether this parser can parse the given file.
     * @param name File name (used for extension checks).
     * @param device QIODevice representing the file content.
     * @return true if the parser can handle this file.
     */
    virtual bool getCanParseFile(const QString &name, QIODevice &device) = 0;

    /**
     * @brief Parses a database file and emits addCard/addSet signals.
     * @param device QIODevice representing the file content.
     */
    virtual void parseFile(QIODevice &device) = 0;

    /**
     * @brief Saves card and set data to a file.
     * @param _formats
     * @param sets Map of sets to save.
     * @param cards Map of cards to save.
     * @param fileName Target file path.
     * @param sourceUrl Optional source URL of the database.
     * @param sourceVersion Optional version string of the source.
     * @return true if save succeeded.
     */
    virtual bool saveToFile(FormatRulesNameMap _formats,
                            SetNameMap sets,
                            CardNameMap cards,
                            const QString &fileName,
                            const QString &sourceUrl = "unknown",
                            const QString &sourceVersion = "unknown") = 0;

    /** @brief Clears the cached global set list. */
    static void clearSetlist();

protected:
    /** @brief Cached global list of sets shared between all parsers. */
    static SetNameMap sets;
    ICardSetPriorityController *cardSetPriorityController;

    /**
     * @brief Internal helper to add a set to the global set cache.
     * @param setName Short set name.
     * @param longName Optional full name.
     * @param setType Optional set type string.
     * @param releaseDate Optional release date.
     * @param priority Optional priority (fallback if not specified).
     * @return Pointer to the added or existing CardSet instance.
     */
    CardSetPtr internalAddSet(const QString &setName,
                              const QString &longName = "",
                              const QString &setType = "",
                              const QDate &releaseDate = QDate(),
                              const CardSet::Priority priority = CardSet::PriorityFallback);

signals:
    /** Emitted when a card is loaded from the database. */
    void addCard(CardInfoPtr card);

    /** Emitted when a set is loaded from the database. */
    void addSet(CardSetPtr set);

    void addFormat(FormatRulesPtr format);
};

Q_DECLARE_INTERFACE(ICardDatabaseParser, "ICardDatabaseParser")

#endif
