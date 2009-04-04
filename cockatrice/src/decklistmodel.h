#ifndef DECKLISTMODEL_H
#define DECKLISTMODEL_H

#include <QAbstractListModel>
#include <QList>

class CardDatabase;

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
	DeckListModel(CardDatabase *_db, QObject *parent = 0);
	~DeckListModel();
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	bool loadFromFile(const QString &fileName);
	bool saveToFile(const QString &fileName);
	DecklistRow *getRow(int row) const;
	void cleanList();
private:
	CardDatabase *db;
	QList<DecklistRow *> deckList;
	void cacheCardPictures();
};

#endif
