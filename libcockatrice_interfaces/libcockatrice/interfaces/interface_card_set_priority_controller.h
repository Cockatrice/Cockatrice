#ifndef COCKATRICE_INTERFACE_CARD_SET_PRIORITY_CONTROLLER_H
#define COCKATRICE_INTERFACE_CARD_SET_PRIORITY_CONTROLLER_H

#include <QString>

class ICardSetPriorityController
{
public:
    struct SetSaveData
    {
        QString shortName;
        unsigned int sortKey;
        bool enabled;
    };

    /**
     * @brief Bundled per-set priority options, returned in a single call.
     *
     * Reading these individually (getSortKey/isEnabled/isKnown) is cheap for the
     * noop controller but extremely expensive when backed by QSettings, which
     * re-opens and re-parses the whole .ini on every access. Callers that need
     * more than one option for the same set (notably CardSet construction during
     * database load) should use getSetOptions() to avoid that cost.
     */
    struct SetOptions
    {
        unsigned int sortKey = 0;
        bool enabled = true;
        bool isKnown = true;
    };

    virtual ~ICardSetPriorityController() = default;

    virtual void setSortKey(QString shortName, unsigned int sortKey) = 0;
    virtual void setEnabled(QString shortName, bool enabled) = 0;
    virtual void setIsKnown(QString shortName, bool isknown) = 0;

    virtual unsigned int getSortKey(QString shortName) const = 0;
    virtual bool isEnabled(QString shortName) const = 0;
    virtual bool isKnown(QString shortName) const = 0;

    /**
     * @brief Returns the bundled priority options for a set in a single call.
     * @param shortName The set's short name.
     * @return The sort key, enabled and known flags for the set.
     */
    virtual SetOptions getSetOptions(QString shortName) const = 0;

    virtual void saveSets(const QVector<SetSaveData> &data) = 0;
};

#endif // COCKATRICE_INTERFACE_CARD_SET_PRIORITY_CONTROLLER_H
