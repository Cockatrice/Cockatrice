#include "deck_list_sort_filter_proxy_model.h"

#include "deck_list_model.h"

bool DeckListSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    auto *src = sourceModel();

    // Inner nodes? -> sort alphabetically by column 1
    bool leftIsCard = src->data(left, Qt::UserRole + 1).toBool();
    bool rightIsCard = src->data(right, Qt::UserRole + 1).toBool();

    if (!leftIsCard || !rightIsCard) {
        QString lName = src->data(left.siblingAtColumn(1), Qt::EditRole).toString();
        QString rName = src->data(right.siblingAtColumn(1), Qt::EditRole).toString();
        return lName.localeAwareCompare(rName) < 0;
    }

    // Both are cards -> apply sort criteria
    auto *lNode = static_cast<DecklistModelCardNode *>(left.internalPointer());
    auto *rNode = static_cast<DecklistModelCardNode *>(right.internalPointer());

    CardInfoPtr lInfo = CardDatabaseManager::query()->guessCard({lNode->getName()}).getCardPtr();
    CardInfoPtr rInfo = CardDatabaseManager::query()->guessCard({rNode->getName()}).getCardPtr();

    // Example: multiple tie-break criteria (colors > cmc > name)
    for (const QString &crit : sortCriteria) {
        if (crit == "name") {
            QString ln = lNode->getName();
            QString rn = rNode->getName();
            int cmp = ln.localeAwareCompare(rn);
            if (cmp != 0)
                return cmp < 0;
        } else if (crit == "cmc") {
            int lc = lInfo ? lInfo->getCmc().toInt() : 0;
            int rc = rInfo ? rInfo->getCmc().toInt() : 0;
            if (lc != rc)
                return lc < rc;
        } else if (crit == "colors") {
            QString lr = lInfo ? lInfo->getColors() : QString();
            QString rr = rInfo ? rInfo->getColors() : QString();
            int cmp = lr.localeAwareCompare(rr);
            if (cmp != 0)
                return cmp < 0;
        } else if (crit == "maintype") {
            QString lr = lInfo ? lInfo->getMainCardType() : QString();
            QString rr = rInfo ? rInfo->getMainCardType() : QString();
            int cmp = lr.localeAwareCompare(rr);
            if (cmp != 0)
                return cmp < 0;
        }
    }

    return false;
}
