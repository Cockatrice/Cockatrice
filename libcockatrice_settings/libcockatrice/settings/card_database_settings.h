/**
 * @file card_database_settings.h
 * @ingroup CardDatabase
 * @ingroup CardSettings
 */
//! \todo Document this file.

#ifndef CARDDATABASESETTINGS_H
#define CARDDATABASESETTINGS_H

#include "settings_manager.h"

#include <QHash>
#include <QMutex>
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

    SetOptions getSetOptions(QString shortName) const override;

    void saveSets(const QVector<SetSaveData> &data) override;

private:
    explicit CardDatabaseSettings(const QString &settingPath, QObject *parent = nullptr);

    // In-memory cache of set priority options. QSettings re-opens and re-parses
    // the whole .ini on every access, which is catastrophic when CardSet
    // construction calls getSortKey/isEnabled/isKnown once per set during the
    // database load (thousands of file parses). The cache is populated from a
    // single QSettings session and kept consistent with the writes below.
    mutable QMutex setOptionsMutex;
    mutable QHash<QString, SetOptions> setOptionsCache;
    mutable bool setOptionsLoaded = false;

    void ensureSetOptionsLoaded() const;
};

#endif // CARDDATABASESETTINGS_H
