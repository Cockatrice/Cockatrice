/**
 * @file card_database_settings.h
 * @ingroup CardDatabase
 * @ingroup CardSettings
 * @brief TODO: Document this.
 */

#ifndef CARDDATABASESETTINGS_H
#define CARDDATABASESETTINGS_H

#include "settings_manager.h"

#include <libcockatrice/interfaces/interface_card_set_priority_controller.h>

class CardDatabaseSettings : public SettingsManager, public ICardSetPriorityController
{
    Q_OBJECT
    friend class SettingsCache;

public:
    void setSortKey(QString shortName, unsigned int sortKey) override;
    void setEnabled(QString shortName, bool enabled) override;
    void setIsKnown(QString shortName, bool isknown) override;

    unsigned int getSortKey(QString shortName) const override;
    bool isEnabled(QString shortName) const override;
    bool isKnown(QString shortName) const override;

    void saveSets(const QVector<SetSaveData> &data) override;

private:
    explicit CardDatabaseSettings(const QString &settingPath, QObject *parent = nullptr);
};

#endif // CARDDATABASESETTINGS_H
