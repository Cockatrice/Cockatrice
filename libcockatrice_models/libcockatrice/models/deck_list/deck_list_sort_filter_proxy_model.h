/**
 * @file deck_list_sort_filter_proxy_model.h
 * @ingroup DeckEditorCardGroupWidgets
 */
//! \todo Document this file.

#ifndef COCKATRICE_DECK_LIST_SORT_FILTER_PROXY_MODEL_H
#define COCKATRICE_DECK_LIST_SORT_FILTER_PROXY_MODEL_H

#include "../natural_sort_collator.h"

#include <QCollator>
#include <QList>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringList>
#include <qtmetamacros.h>

class QObject;

class DeckListSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit DeckListSortFilterProxyModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent), collator(NaturalSort::makeNaturalSortCollator())
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
    QCollator collator;
};

#endif // COCKATRICE_DECK_LIST_SORT_FILTER_PROXY_MODEL_H
