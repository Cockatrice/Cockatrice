#ifndef MAIN_H
#define MAIN_H

#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/parser/cockatrice_xml_4.h>

class CardDatabaseConverter : public CardDatabase
{
public:
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
