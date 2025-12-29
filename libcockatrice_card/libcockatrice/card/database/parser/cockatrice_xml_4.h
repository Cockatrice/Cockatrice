#ifndef COCKATRICE_XML4_H
#define COCKATRICE_XML4_H

#include "card_database_parser.h"

#include <QLoggingCategory>
#include <QXmlStreamReader>
#include <libcockatrice/interfaces/interface_card_preference_provider.h>

inline Q_LOGGING_CATEGORY(CockatriceXml4Log, "cockatrice_xml.xml_4_parser");

/**
 * @class CockatriceXml4Parser
 * @ingroup CardDatabase
 * @brief Parses version 4 of the Cockatrice XML Schema.
 *
 * This parser reads a Cockatrice XML4 database and emits CardInfoPtr
 * and CardSetPtr objects. Card properties are read inside <prop> blocks,
 * making the parser more extensible and schema-compliant.
 *
 * @note Differences from v3:
 * - Card properties are stored in <prop> blocks as a QVariantHash.
 * - Sets can include a <priority> element.
 * - Supports user preferences via ICardPreferenceProvider (e.g., skipping rebalanced cards).
 * - Related cards support persistent relations and multiple attach types (e.g., transform).
 * - More robust serialization; easier to extend schema in the future.
 */
class CockatriceXml4Parser : public ICardDatabaseParser
{
    Q_OBJECT
public:
    explicit CockatriceXml4Parser(ICardPreferenceProvider *cardPreferenceProvider,
                                  ICardSetPriorityController *cardSetPriorityController);
    ~CockatriceXml4Parser() override = default;

    /**
     * @brief Determines if the parser can handle this file.
     * @param name File name.
     * @param device Open QIODevice containing the XML.
     * @return True if the file is a Cockatrice XML4 database.
     */
    bool getCanParseFile(const QString &name, QIODevice &device) override;

    /**
     * @brief Parse the XML database.
     * @param device Open QIODevice positioned at start of file.
     */
    void parseFile(QIODevice &device) override;

    /**
     * @brief Save sets and cards back to an XML4 file.
     */
    bool saveToFile(FormatRulesNameMap _formats,
                    SetNameMap _sets,
                    CardNameMap cards,
                    const QString &fileName,
                    const QString &sourceUrl = "unknown",
                    const QString &sourceVersion = "unknown") override;

private:
    ICardPreferenceProvider *cardPreferenceProvider; ///< Interface to handle user preferences

    /**
     * @brief Loads a generic <prop> block from a <card> element.
     * @param xml The open QXmlStreamReader positioned at a <prop> element.
     * @return A QVariantHash mapping property names to values.
     */
    QVariantHash loadCardPropertiesFromXml(QXmlStreamReader &xml);

    /**
     * @brief Load all <card> elements from the XML stream.
     * @param xml The open QXmlStreamReader positioned at the <cards> element.
     * Honors the user's preference regarding rebalanced cards.
     */
    void loadCardsFromXml(QXmlStreamReader &xml);

    void loadFormats(QXmlStreamReader &xml);
    /**
     * @brief Load all <set> elements from the XML stream.
     * @param xml The open QXmlStreamReader positioned at the <sets> element.
     * Parses <set> nodes including priority information.
     */
    void loadSetsFromXml(QXmlStreamReader &xml);
};

#endif