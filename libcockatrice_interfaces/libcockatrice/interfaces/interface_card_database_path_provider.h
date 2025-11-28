#ifndef COCKATRICE_INTERFACE_CARD_DATABASE_PATH_PROVIDER_H
#define COCKATRICE_INTERFACE_CARD_DATABASE_PATH_PROVIDER_H
#include <QObject>

class ICardDatabasePathProvider : public QObject
{
    Q_OBJECT

public:
    virtual ~ICardDatabasePathProvider() = default;

    [[nodiscard]] virtual QString getCardDatabasePath() const = 0;
    [[nodiscard]] virtual QString getCustomCardDatabasePath() const = 0;
    [[nodiscard]] virtual QString getTokenDatabasePath() const = 0;
    [[nodiscard]] virtual QString getSpoilerCardDatabasePath() const = 0;

signals:
    void cardDatabasePathChanged();
};

#endif // COCKATRICE_INTERFACE_CARD_DATABASE_PATH_PROVIDER_H
