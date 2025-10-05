#include "card_set_list.h"

class CardSetList::KeyCompareFunctor
{
public:
    inline bool operator()(const CardSetPtr &a, const CardSetPtr &b) const
    {
        if (a.isNull() || b.isNull()) {
            // qCWarning(CardInfoLog) << "SetList::KeyCompareFunctor a or b is null";
            return false;
        }

        return a->getSortKey() < b->getSortKey();
    }
};

void CardSetList::sortByKey()
{
    std::sort(begin(), end(), KeyCompareFunctor());
}

int CardSetList::getEnabledSetsNum()
{
    int num = 0;
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);
        if (set && set->getEnabled()) {
            ++num;
        }
    }
    return num;
}

int CardSetList::getUnknownSetsNum()
{
    int num = 0;
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);
        if (set && !set->getIsKnown() && !set->getIsKnownIgnored()) {
            ++num;
        }
    }
    return num;
}

QStringList CardSetList::getUnknownSetsNames()
{
    QStringList sets = QStringList();
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);
        if (set && !set->getIsKnown() && !set->getIsKnownIgnored()) {
            sets << set->getShortName();
        }
    }
    return sets;
}

void CardSetList::enableAllUnknown()
{
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);
        if (set && !set->getIsKnown() && !set->getIsKnownIgnored()) {
            set->setIsKnown(true);
            set->setEnabled(true);
        } else if (set && set->getIsKnownIgnored() && !set->getEnabled()) {
            set->setEnabled(true);
        }
    }
}

void CardSetList::enableAll()
{
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);

        if (set == nullptr) {
            // qCWarning(CardInfoLog) << "enabledAll has null";
            continue;
        }

        if (!set->getIsKnownIgnored()) {
            set->setIsKnown(true);
        }

        set->setEnabled(true);
    }
}

void CardSetList::markAllAsKnown()
{
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);
        if (set && !set->getIsKnown() && !set->getIsKnownIgnored()) {
            set->setIsKnown(true);
            set->setEnabled(false);
        } else if (set && set->getIsKnownIgnored() && !set->getEnabled()) {
            set->setEnabled(true);
        }
    }
}

void CardSetList::guessSortKeys()
{
    defaultSort();
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);
        if (set.isNull()) {
            // qCWarning(CardInfoLog) << "guessSortKeys set is null";
            continue;
        }
        set->setSortKey(i);
    }
}

void CardSetList::defaultSort()
{
    std::sort(begin(), end(), [](const CardSetPtr &a, const CardSetPtr &b) {
        // Sort by priority, then by release date, then by short name
        if (a->getPriority() != b->getPriority()) {
            return a->getPriority() < b->getPriority(); // lowest first
        } else if (a->getReleaseDate() != b->getReleaseDate()) {
            return a->getReleaseDate() > b->getReleaseDate(); // most recent first
        } else {
            return a->getShortName() < b->getShortName(); // alphabetically
        }
    });
}