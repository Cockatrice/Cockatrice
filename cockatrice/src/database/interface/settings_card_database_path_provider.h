#ifndef SETTINGS_CARD_DATABASE_PATH_PROVIDER_H
#define SETTINGS_CARD_DATABASE_PATH_PROVIDER_H

#include "../../client/settings/cache_settings.h"

#include <libcockatrice/interfaces/interface_card_database_path_provider.h>

class SettingsCardDatabasePathProvider : public ICardDatabasePathProvider
{
    Q_OBJECT

public:
    explicit SettingsCardDatabasePathProvider(QObject *parent = nullptr) : ICardDatabasePathProvider(parent)
    {
        connect(&SettingsCache::instance().paths(), &PathsSettings::cardDatabasePathChanged, this,
                &ICardDatabasePathProvider::cardDatabasePathChanged);
    }

    [[nodiscard]] QString getCardDatabasePath() const override
    {
        return SettingsCache::instance().paths().getCardDatabasePath();
    }

    [[nodiscard]] QString getCustomCardDatabasePath() const override
    {
        return SettingsCache::instance().paths().getCustomCardDatabasePath();
    }

    [[nodiscard]] QString getTokenDatabasePath() const override
    {
        return SettingsCache::instance().paths().getTokenDatabasePath();
    }

    [[nodiscard]] virtual QString getSpoilerCardDatabasePath() const override
    {
        return SettingsCache::instance().paths().getSpoilerCardDatabasePath();
    }
};

#endif // SETTINGS_CARD_DATABASE_PATH_PROVIDER_H
