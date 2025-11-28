/**
 * @file filter_tree_model.h
 * @ingroup CardDatabaseModelFilters
 * @brief TODO: Document this.
 */

#ifndef FILTERTREEMODEL_H
#define FILTERTREEMODEL_H

#include <QAbstractItemModel>
#include <libcockatrice/filters/filter_card.h>

class FilterTree;
class CardFilter;
class FilterTreeNode;

class FilterTreeModel : public QAbstractItemModel
{
    Q_OBJECT
private:
    FilterTree *fTree;

public slots:
    void addFilter(const CardFilter *f);
    void removeFilter(const CardFilter *f);
    void clearFiltersOfType(CardFilter::Attr filterType);
    [[nodiscard]] QList<const CardFilter *> getFiltersOfType(CardFilter::Attr filterType) const;
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
