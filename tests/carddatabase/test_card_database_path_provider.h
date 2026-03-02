#ifndef COCKATRICE_TEST_CARD_DATABASE_PATH_PROVIDER_H
#define COCKATRICE_TEST_CARD_DATABASE_PATH_PROVIDER_H

#include <libcockatrice/interfaces/interface_card_database_path_provider.h>

class TestCardDatabasePathProvider : public ICardDatabasePathProvider
{

public:
    QString getCardDatabasePath() const override
    {
        return QString("%1/cards.xml").arg(CARDDB_DATADIR);
    }
    QString getCustomCardDatabasePath() const override
    {
        return QString("%1/customsets/").arg(CARDDB_DATADIR);
    }
    QString getTokenDatabasePath() const override
    {
        return QString("%1/tokens.xml").arg(CARDDB_DATADIR);
    }
    QString getSpoilerCardDatabasePath() const override
    {
        return QString("%1/spoiler.xml").arg(CARDDB_DATADIR);
    }
};

#endif // COCKATRICE_TEST_CARD_DATABASE_PATH_PROVIDER_H
