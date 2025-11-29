#include "deck_list_sort_filter_proxy_model.h"

#include "deck_list_model.h"

bool DeckListSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const auto *src = sourceModel();

    // Inner nodes? -> sort alphabetically by column 1
    const bool leftIsCard = src->data(left, Qt::UserRole + 1).toBool();
    const bool rightIsCard = src->data(right, Qt::UserRole + 1).toBool();

    if (!leftIsCard || !rightIsCard) {
        const QString lName = src->data(left.siblingAtColumn(1), Qt::EditRole).toString();
        const QString rName = src->data(right.siblingAtColumn(1), Qt::EditRole).toString();
        return lName.localeAwareCompare(rName) < 0;
    }

    // Both are cards -> apply sort criteria
    const auto *lNode = static_cast<DecklistModelCardNode *>(left.internalPointer());
    const auto *rNode = static_cast<DecklistModelCardNode *>(right.internalPointer());

    const CardInfoPtr lInfo = CardDatabaseManager::query()->guessCard({lNode->getName()}).getCardPtr();
    const CardInfoPtr rInfo = CardDatabaseManager::query()->guessCard({rNode->getName()}).getCardPtr();

    // Example: multiple tie-break criteria (colors > cmc > name)
    for (const QString &crit : sortCriteria) {
        if (crit == "name") {
            QString ln = lNode->getName();
            QString rn = rNode->getName();
            const int cmp = ln.localeAwareCompare(rn);
            if (cmp != 0)
                return cmp < 0;
        } else if (crit == "cmc") {
            const int lc = lInfo ? lInfo->getCmc().toInt() : 0;
            const int rc = rInfo ? rInfo->getCmc().toInt() : 0;
            if (lc != rc)
                return lc < rc;
        } else if (crit == "colors") {
            QString lr = lInfo ? lInfo->getColors() : QString();
            QString rr = rInfo ? rInfo->getColors() : QString();
            const int cmp = lr.localeAwareCompare(rr);
            if (cmp != 0)
                return cmp < 0;
        } else if (crit == "maintype") {
            QString lr = lInfo ? lInfo->getMainCardType() : QString();
            QString rr = rInfo ? rInfo->getMainCardType() : QString();
            const int cmp = lr.localeAwareCompare(rr);
            if (cmp != 0)
                return cmp < 0;
        }
    }

    return false;
}
