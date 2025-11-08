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
        return parser.saveToFile(sets, cards, fileName);
    }
};

#endif
