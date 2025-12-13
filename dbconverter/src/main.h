#ifndef MAIN_H
#define MAIN_H

#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/parser/cockatrice_xml_4.h>
#include <libcockatrice/interfaces/noop_card_preference_provider.h>

static const QList<AllowedCount> kConstructedCounts = {{4, "legal"}, {0, "banned"}};

static const QList<AllowedCount> kSingletonCounts = {{1, "legal"}, {0, "banned"}};

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
        anyNumberAllowed.value = "A deck can have any number of";

        ExceptionRule mayContainAnyNumber;
        mayContainAnyNumber.conditions.append(anyNumberAllowed);

        // Map to store default rules
        FormatRulesNameMap defaultFormatRulesNameMap;

        // ----------------- Helper lambda to create format -----------------
        auto makeFormat = [&](const QString &name, int minDeck = 60, int maxDeck = -1, int maxSideboardSize = 15,
                              const QList<AllowedCount> &allowedCounts = kConstructedCounts) -> FormatRulesPtr {
            FormatRulesPtr f(new FormatRules);
            f->formatName = name;
            f->allowedCounts = allowedCounts;
            f->minDeckSize = minDeck;
            f->maxDeckSize = maxDeck;
            f->maxSideboardSize = maxSideboardSize;
            f->exceptions.append(basicLands);
            f->exceptions.append(mayContainAnyNumber);
            defaultFormatRulesNameMap.insert(name.toLower(), f);
            return f;
        };

        // ----------------- Standard formats -----------------
        makeFormat("Standard");
        makeFormat("Modern");
        makeFormat("Legacy");
        makeFormat("Pioneer");
        makeFormat("Historic");
        makeFormat("Timeless");
        makeFormat("Future");
        makeFormat("OldSchool");
        makeFormat("Premodern");
        makeFormat("Pauper");
        makeFormat("Penny");

        // ----------------- Singleton formats -----------------
        makeFormat("Commander", 100, 100, 15, kSingletonCounts);
        makeFormat("Duel", 100, 100, 15, kSingletonCounts);
        makeFormat("Brawl", 60, 60, 15, kSingletonCounts);
        makeFormat("StandardBrawl", 60, 60, 15, kSingletonCounts);
        makeFormat("Oathbreaker", 60, 60, 15, kSingletonCounts);
        makeFormat("PauperCommander", 100, 100, 15, kSingletonCounts);
        makeFormat("Predh", 100, 100, 15, kSingletonCounts);

        // ----------------- Restricted formats -----------------
        makeFormat("Vintage", 60, -1, 15, {{4, "legal"}, {1, "restricted"}, {0, "banned"}});

        return defaultFormatRulesNameMap;
    }
};

#endif
