#ifndef COCKATRICE_XML3_H
#define COCKATRICE_XML3_H

#include "card_database_parser.h"

#include <QLoggingCategory>
#include <QXmlStreamReader>

inline Q_LOGGING_CATEGORY(CockatriceXml3Log, "cockatrice_xml.xml_3_parser");

/**
 * @class CockatriceXml3Parser
 * @ingroup CardDatabase
 * @brief Parses version 3 of the Cockatrice XML Schema.
 *
 * This parser reads a Cockatrice XML3 database and emits CardInfoPtr
 * and CardSetPtr objects. All card properties are read individually.
 *
 * @note Differences from v4:
 * - No <prop> block; properties are hardcoded (manacost, cmc, type, pt, loyalty, etc.).
 * - No set priority field.
 * - No support for rebalanced cards or preferences.
 * - Related cards support only attach, exclude, variable, and count attributes.
 */
class CockatriceXml3Parser : public ICardDatabaseParser
{
    Q_OBJECT
public:
    CockatriceXml3Parser(ICardSetPriorityController *cardSetPriorityController);
    ~CockatriceXml3Parser() override = default;

    /**
     * @brief Determines if the parser can handle this file.
     * @param name File name.
     * @param device Open QIODevice containing the XML.
     * @return True if the file is a Cockatrice XML3 database.
     */
    bool getCanParseFile(const QString &name, QIODevice &device) override;

    /**
     * @brief Parse the XML database.
     * @param device Open QIODevice positioned at start of file.
     */
    void parseFile(QIODevice &device) override;

    /**
     * @brief Save sets and cards back to an XML3 file.
     */
    bool saveToFile(FormatRulesNameMap _formats,
                    SetNameMap _sets,
                    CardNameMap cards,
                    const QString &fileName,
                    const QString &sourceUrl = "unknown",
                    const QString &sourceVersion = "unknown") override;

private:
    /**
     * @brief Load all <card> elements from the XML stream.
     * @param xml The open QXmlStreamReader positioned at the <cards> element.
     * Parses each <card> node and emits addCard signals for each CardInfoPtr created.
     */
    void loadCardsFromXml(QXmlStreamReader &xml);

    /**
     * @brief Load all <set> elements from the XML stream.
     * @param xml The open QXmlStreamReader positioned at the <sets> element.
     * Parses each <set> node and adds them to the shared set cache.
     */
    void loadSetsFromXml(QXmlStreamReader &xml);

    /**
     * @brief Extracts the main card type from a full type string.
     * @param type The full type string (e.g., "Legendary Artifact Creature - Golem")
     * @return The primary type (e.g., "Creature").
     */
    QString getMainCardType(QString &type);
};

#endif