#ifndef MAIN_H
#define MAIN_H

#include "../../cockatrice/src/game/cards/card_database.h"
#include "../../cockatrice/src/game/cards/card_database_parser/cockatrice_xml_4.h"

class CardDatabaseConverter : public CardDatabase
{
public:
    LoadStatus loadCardDatabase(const QString &path)
    {
        return CardDatabase::loadCardDatabase(path);
    }

    bool saveCardDatabase(const QString &fileName)
    {
        CockatriceXml4Parser parser;
        return parser.saveToFile(sets, cards, fileName);
    }
};

#endif
