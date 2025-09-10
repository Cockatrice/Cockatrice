#ifndef MAIN_H
#define MAIN_H

#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/parser/cockatrice_xml_4.h>
#include <libcockatrice/interfaces/noop_card_preference_provider.h>

class CardDatabaseConverter : public CardDatabase
{
public:
    explicit CardDatabaseConverter()
    {
        // Replace querier with one that ignores SettingsCache
        delete querier;
        querier = new CardDatabaseQuerier(this, this, new NoopCardPreferenceProvider());
    };

    LoadStatus loadCardDatabase(const QString &path)
    {
        return loader->loadCardDatabase(path);
    }

    bool saveCardDatabase(const QString &fileName)
    {
        CockatriceXml4Parser parser(new NoopCardPreferenceProvider());

        return parser.saveToFile(createDefaultMagicFormats(), sets, cards, fileName);
    }

    FormatRulesNameMap createDefaultMagicFormats()
    {
        // Predefined common exceptions
        CardCondition superTypeIsBasic;
        superTypeIsBasic.field = "type";
        superTypeIsBasic.matchType = "contains";
        superTypeIsBasic.value = "Basic Land";

        ExceptionRule basicLands;
        basicLands.conditions.append(superTypeIsBasic);

        CardCondition anyNumberAllowed;
        anyNumberAllowed.field = "text";
        anyNumberAllowed.matchType = "contains";
        anyNumberAllowed.value = "may contain any number of";

        ExceptionRule mayContainAnyNumber;
        mayContainAnyNumber.conditions.append(anyNumberAllowed);

        // Map to store default rules
        FormatRulesNameMap defaultFormatRulesNameMap;

        // ----------------- Helper lambda to create format -----------------
        auto makeFormat = [&](const QString &name, int maxCopies = 4, int minDeck = 60, int maxDeck = -1,
                              int maxSideboardSize = 15) -> FormatRulesPtr {
            FormatRulesPtr f(new FormatRules);
            f->formatName = name;
            f->maxCopies = maxCopies;
            f->minDeckSize = minDeck;
            f->maxDeckSize = maxDeck;
            f->maxSideboardSize = maxSideboardSize;
            f->exceptions.append(basicLands);
            f->exceptions.append(mayContainAnyNumber);
            defaultFormatRulesNameMap.insert(name.toLower(), f);
            return f;
        };

        // ----------------- Standard formats -----------------
        makeFormat("Alchemy", 4, 60);
        makeFormat("Brawl", 1, 60, 60);
        makeFormat("Commander", 1, 100, 100);
        makeFormat("Duel", 1, 100, 100);
        makeFormat("Future");
        makeFormat("Gladiator", 4, 60, 60, 0);
        makeFormat("Historic");
        makeFormat("Legacy");
        makeFormat("Modern");
        makeFormat("Oathbreaker", 1, 60, 60);
        makeFormat("OldSchool");
        makeFormat("Pauper");
        makeFormat("PauperCommander", 1, 100, 100);
        makeFormat("Penny");
        makeFormat("Pioneer");
        makeFormat("Predh", 1, 100, 100);
        makeFormat("Premodern");
        makeFormat("Standard");
        makeFormat("StandardBrawl", 1, 60, 60);
        makeFormat("Timeless");
        makeFormat("Vintage");

        return defaultFormatRulesNameMap;
    }
};

#endif
