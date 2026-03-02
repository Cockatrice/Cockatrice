#ifndef COCKATRICE_NOOP_CARD_DATABASE_PATH_PROVIDER_H
#define COCKATRICE_NOOP_CARD_DATABASE_PATH_PROVIDER_H
#include "interface_card_database_path_provider.h"

class NoopCardDatabasePathProvider : public ICardDatabasePathProvider
{
public:
    [[nodiscard]] QString getCardDatabasePath() const override
    {
        return "";
    }
    [[nodiscard]] QString getCustomCardDatabasePath() const override
    {
        return "";
    }
    [[nodiscard]] QString getTokenDatabasePath() const override
    {
        return "";
    }
    [[nodiscard]] QString getSpoilerCardDatabasePath() const override
    {
        return "";
    }
};

#endif // COCKATRICE_NOOP_CARD_DATABASE_PATH_PROVIDER_H
