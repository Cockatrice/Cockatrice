#ifndef FILTERLISTMODEL_H
#define FILTERLISTMODEL_H

#include <QAbstractItemModel>

class FilterList;
class CardFilter;
class FilterListNode;

class FilterListModel : public QAbstractItemModel {
	Q_OBJECT
private:
	FilterList *filterList;

public slots:
	void addFilter(const CardFilter *f);

private slots:
	void proxyBeginInsertRow(const FilterListNode *, int);
	void proxyEndInsertRow(const FilterListNode *node, int i);

private:
	FilterListNode *indexToNode(const QModelIndex &idx) const;
	QModelIndex nodeIndex(const FilterListNode *node, int row, int column) const;

public:
	FilterListModel(QObject *parent = 0);
	~FilterListModel();
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &/*parent*/ = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value,
					int role = Qt::EditRole);
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QModelIndex parent(const QModelIndex &ind) const;
	QModelIndex index(int row, int column, 
					const QModelIndex &parent) const;
	bool removeRows(int row, int count, const QModelIndex & parent);
};

#endif
