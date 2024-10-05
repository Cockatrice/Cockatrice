#ifndef MAIN_H
#define MAIN_H

#include "../../cockatrice/src/card_database_parser/cockatricexml4.h"
#include "../../cockatrice/src/carddatabase.h"

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
