#ifndef COCKATRICE_INTERFACE_CARD_DATABASE_PATH_PROVIDER_H
#define COCKATRICE_INTERFACE_CARD_DATABASE_PATH_PROVIDER_H

class ICardDatabasePathProvider
{

public:
    virtual ~ICardDatabasePathProvider() = default;

    [[nodiscard]] virtual QString getCardDatabasePath() const = 0;
    [[nodiscard]] virtual QString getCustomCardDatabasePath() const = 0;
    [[nodiscard]] virtual QString getTokenDatabasePath() const = 0;
    [[nodiscard]] virtual QString getSpoilerCardDatabasePath() const = 0;
};

#endif // COCKATRICE_INTERFACE_CARD_DATABASE_PATH_PROVIDER_H
