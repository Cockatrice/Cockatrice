#ifndef DECKLISTMODEL_H
#define DECKLISTMODEL_H

#include <QAbstractListModel>
#include <QList>

class DecklistRow {
private:
	int number;
	QString card;
	bool sideboard;
public:
	DecklistRow(int _number, const QString &_card, bool _sideboard) : number(_number), card(_card), sideboard(_sideboard) { }
	int getNumber() const { return number; }
	QString getCard() const { return card; }
	bool isSideboard() const { return sideboard; }
};

class DeckListModel : public QAbstractListModel {
	Q_OBJECT
public:
	DeckListModel(QObject *parent = 0)
		: QAbstractListModel(parent) { }
	~DeckListModel();
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	void loadFromFile(const QString &fileName);
	DecklistRow *getRow(int row) const;
private:
	QList<DecklistRow *> deckList;
	void cleanList();
};

#endif
