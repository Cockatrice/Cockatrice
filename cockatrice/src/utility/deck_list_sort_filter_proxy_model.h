#ifndef COCKATRICE_DECK_LIST_SORT_FILTER_PROXY_MODEL_H
#define COCKATRICE_DECK_LIST_SORT_FILTER_PROXY_MODEL_H

#include "../database/card_database_manager.h"

#include <QSortFilterProxyModel>

class DeckListSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit DeckListSortFilterProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent)
    {
    }

    void setSortCriteria(const QStringList &criteria)
    {
        sortCriteria = criteria;
        invalidate(); // re-sort
    }

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    QStringList sortCriteria;
};

#endif // COCKATRICE_DECK_LIST_SORT_FILTER_PROXY_MODEL_H
