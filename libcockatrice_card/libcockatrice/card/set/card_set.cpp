#include "card_set.h"

#include <libcockatrice/settings/cache_settings.h>

const char *CardSet::TOKENS_SETNAME = "TK";

CardSet::CardSet(const QString &_shortName,
                 const QString &_longName,
                 const QString &_setType,
                 const QDate &_releaseDate,
                 const CardSet::Priority _priority)
    : shortName(_shortName), longName(_longName), releaseDate(_releaseDate), setType(_setType), priority(_priority)
{
    loadSetOptions();
}

CardSetPtr CardSet::newInstance(const QString &_shortName,
                                const QString &_longName,
                                const QString &_setType,
                                const QDate &_releaseDate,
                                const Priority _priority)
{
    CardSetPtr ptr(new CardSet(_shortName, _longName, _setType, _releaseDate, _priority));
    // ptr->setSmartPointer(ptr);
    return ptr;
}

QString CardSet::getCorrectedShortName() const
{
    // For Windows machines.
    QSet<QString> invalidFileNames;
    invalidFileNames << "CON"
                     << "PRN"
                     << "AUX"
                     << "NUL"
                     << "COM1"
                     << "COM2"
                     << "COM3"
                     << "COM4"
                     << "COM5"
                     << "COM6"
                     << "COM7"
                     << "COM8"
                     << "COM9"
                     << "LPT1"
                     << "LPT2"
                     << "LPT3"
                     << "LPT4"
                     << "LPT5"
                     << "LPT6"
                     << "LPT7"
                     << "LPT8"
                     << "LPT9";

    return invalidFileNames.contains(shortName) ? shortName + "_" : shortName;
}

void CardSet::loadSetOptions()
{
    sortKey = SettingsCache::instance().cardDatabase().getSortKey(shortName);
    enabled = SettingsCache::instance().cardDatabase().isEnabled(shortName);
    isknown = SettingsCache::instance().cardDatabase().isKnown(shortName);
}

void CardSet::setSortKey(unsigned int _sortKey)
{
    sortKey = _sortKey;
    SettingsCache::instance().cardDatabase().setSortKey(shortName, _sortKey);
}

void CardSet::setEnabled(bool _enabled)
{
    enabled = _enabled;
    SettingsCache::instance().cardDatabase().setEnabled(shortName, _enabled);
}

void CardSet::setIsKnown(bool _isknown)
{
    isknown = _isknown;
    SettingsCache::instance().cardDatabase().setIsKnown(shortName, _isknown);
}