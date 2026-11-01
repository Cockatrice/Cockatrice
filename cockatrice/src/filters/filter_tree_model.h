/**
 * @file filter_tree_model.h
 * @ingroup CardDatabaseModelFilters
 */
//! \todo Document this file.

#ifndef FILTERTREEMODEL_H
#define FILTERTREEMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QModelIndex>
#include <libcockatrice/filters/filter_card.h>
#include <qnamespace.h>
#include <qtmetamacros.h>

class FilterTree;
class FilterTreeNode;
class QObject;

class FilterTreeModel : public QAbstractItemModel
{
    Q_OBJECT
private:
    FilterTree *fTree;

public slots:
    void addFilter(const CardFilter *f);
    void removeFilter(const CardFilter *f);
    void clearFiltersOfType(CardFilter::Attr filterType);
    /**
     * @brief Copies every active filter of one attribute into new CardFilter objects.
     *
     * The caller owns the returned filters. Prefer getFilterTermsOfType() when only
     * the terms are needed.
     */
    [[nodiscard]] QList<const CardFilter *> getFiltersOfType(CardFilter::Attr filterType) const;
    /// @brief Copies every active filter into new CardFilter objects. The caller owns them.
    [[nodiscard]] QList<const CardFilter *> allFilters() const;

private slots:
    void proxyBeginInsertRow(const FilterTreeNode *, int);
    void proxyEndInsertRow(const FilterTreeNode *, int);
    void proxyBeginRemoveRow(const FilterTreeNode *, int);
    void proxyEndRemoveRow(const FilterTreeNode *, int);

private:
    [[nodiscard]] FilterTreeNode *indexToNode(const QModelIndex &idx) const;
    QModelIndex nodeIndex(const FilterTreeNode *node, int row, int column) const;

public:
    FilterTreeModel(QObject *parent = nullptr);
    ~FilterTreeModel() override;
    [[nodiscard]] FilterTree *filterTree() const
    {
        return fTree;
    }

    /**
     * @brief The texts of every active filter of one attribute, without allocating filters.
     *
     * Prefer this over getFiltersOfType() when only the filter terms are needed.
     */
    [[nodiscard]] QStringList getFilterTermsOfType(CardFilter::Attr filterType) const;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex & /*parent*/ = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex &ind) const override;
    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    void clear();
};

#endif
