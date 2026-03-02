/**
 * @file deck_list_sort_filter_proxy_model.h
 * @ingroup DeckEditorCardGroupWidgets
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_DECK_LIST_SORT_FILTER_PROXY_MODEL_H
#define COCKATRICE_DECK_LIST_SORT_FILTER_PROXY_MODEL_H

#include <QSortFilterProxyModel>
#include <libcockatrice/card/database/card_database_manager.h>

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
    [[nodiscard]] bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    QStringList sortCriteria;
};

#endif // COCKATRICE_DECK_LIST_SORT_FILTER_PROXY_MODEL_H
