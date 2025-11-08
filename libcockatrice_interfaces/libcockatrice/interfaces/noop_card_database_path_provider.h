#ifndef COCKATRICE_NOOP_CARD_DATABASE_PATH_PROVIDER_H
#define COCKATRICE_NOOP_CARD_DATABASE_PATH_PROVIDER_H
#include "interface_card_database_path_provider.h"

class NoopCardDatabasePathProvider : public ICardDatabasePathProvider
{
public:
    QString getCardDatabasePath() const override
    {
        return "";
    }
    QString getCustomCardDatabasePath() const override
    {
        return "";
    }
    QString getTokenDatabasePath() const override
    {
        return "";
    }
    QString getSpoilerCardDatabasePath() const override
    {
        return "";
    }
};

#endif // COCKATRICE_NOOP_CARD_DATABASE_PATH_PROVIDER_H
