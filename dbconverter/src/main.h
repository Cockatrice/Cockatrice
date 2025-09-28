#ifndef MAIN_H
#define MAIN_H

#include "../../cockatrice/src/database/card_database.h"
#include "../../cockatrice/src/database/interface/noop_card_preference_provider.h"
#include "../../cockatrice/src/database/parser/cockatrice_xml_4.h"

class CardDatabaseConverter : public CardDatabase
{
public:
    explicit CardDatabaseConverter()
    {
        // Replace querier with one that ignores SettingsCache
        delete querier;
        querier = new CardDatabaseQuerier(this, this, std::make_shared<NoopCardPreferenceProvider>());
    };

    LoadStatus loadCardDatabase(const QString &path)
    {
        return loader->loadCardDatabase(path);
    }

    bool saveCardDatabase(const QString &fileName)
    {
        CockatriceXml4Parser parser;
        return parser.saveToFile(sets, cards, fileName);
    }
};

#endif
