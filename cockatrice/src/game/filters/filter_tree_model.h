#ifndef FILTERTREEMODEL_H
#define FILTERTREEMODEL_H

#include <QAbstractItemModel>

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

private slots:
    void proxyBeginInsertRow(const FilterTreeNode *, int);
    void proxyEndInsertRow(const FilterTreeNode *, int);
    void proxyBeginRemoveRow(const FilterTreeNode *, int);
    void proxyEndRemoveRow(const FilterTreeNode *, int);

private:
    FilterTreeNode *indexToNode(const QModelIndex &idx) const;
    QModelIndex nodeIndex(const FilterTreeNode *node, int row, int column) const;

public:
    FilterTreeModel(QObject *parent = nullptr);
    ~FilterTreeModel();
    FilterTree *filterTree() const
    {
        return fTree;
    }
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex & /*parent*/ = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QModelIndex parent(const QModelIndex &ind) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    bool removeRows(int row, int count, const QModelIndex &parent);
};

#endif
