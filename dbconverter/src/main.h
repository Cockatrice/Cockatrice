#ifndef MAIN_H
#define MAIN_H

#include "carddatabase.h"
#include "carddbparser/cockatricexml4.h"

class CardDatabaseConverter : public CardDatabase
{
public:
	LoadStatus loadCardDatabase(const QString &path) {
		return CardDatabase::loadCardDatabase(path);
	}

	bool saveCardDatabase(const QString &fileName) {
    	CockatriceXml4Parser parser;
    	return parser.saveToFile(sets, cards, fileName);
	}
};

#endif
