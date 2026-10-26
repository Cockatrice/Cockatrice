#include "card_set.h"

#include "libcockatrice/interfaces/interface_card_set_priority_controller.h"

#include <QSet>
#include <utility>

const char *CardSet::TOKENS_SETNAME = "TK";

CardSet::CardSet(ICardSetPriorityController *_priorityController,
                 const QString &_shortName,
                 const QString &_longName,
                 const QString &_setType,
                 const QDate &_releaseDate,
                 const CardSet::Priority _priority)
    : priorityController(std::move(_priorityController)), shortName(_shortName), longName(_longName),
      releaseDate(_releaseDate), setType(_setType), priority(_priority)
{
    loadSetOptions();
}

CardSetPtr CardSet::newInstance(ICardSetPriorityController *_priorityController,
                                const QString &_shortName,
                                const QString &_longName,
                                const QString &_setType,
                                const QDate &_releaseDate,
                                const Priority _priority)
{
    CardSetPtr ptr(new CardSet(_priorityController, _shortName, _longName, _setType, _releaseDate, _priority));
    // ptr->setSmartPointer(ptr);
    return ptr;
}

QString CardSet::getCorrectedShortName() const
{
    // For Windows machines.
    static const QSet<QString> invalidFileNames = {
        QStringLiteral("CON"),  QStringLiteral("PRN"),  QStringLiteral("AUX"),  QStringLiteral("NUL"),
        QStringLiteral("COM1"), QStringLiteral("COM2"), QStringLiteral("COM3"), QStringLiteral("COM4"),
        QStringLiteral("COM5"), QStringLiteral("COM6"), QStringLiteral("COM7"), QStringLiteral("COM8"),
        QStringLiteral("COM9"), QStringLiteral("LPT1"), QStringLiteral("LPT2"), QStringLiteral("LPT3"),
        QStringLiteral("LPT4"), QStringLiteral("LPT5"), QStringLiteral("LPT6"), QStringLiteral("LPT7"),
        QStringLiteral("LPT8"), QStringLiteral("LPT9"),
    };

    return invalidFileNames.contains(shortName) ? shortName + "_" : shortName;
}

void CardSet::loadSetOptions()
{
    const ICardSetPriorityController::SetOptions options = priorityController->getSetOptions(shortName);
    sortKey = options.sortKey;
    enabled = options.enabled;
    isknown = options.isKnown;
}

void CardSet::setSortKey(unsigned int _sortKey)
{
    sortKey = _sortKey;
    priorityController->setSortKey(shortName, _sortKey);
}

void CardSet::setEnabled(bool _enabled)
{
    enabled = _enabled;
    priorityController->setEnabled(shortName, _enabled);
}

void CardSet::setIsKnown(bool _isknown)
{
    isknown = _isknown;
    priorityController->setIsKnown(shortName, _isknown);
}