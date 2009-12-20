#ifndef CARDDATABASEMODEL_H
#define CARDDATABASEMODEL_H

#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QList>
#include <QSet>
#include "carddatabase.h"

class CardDatabaseModel : public QAbstractListModel {
	Q_OBJECT
public:
	CardDatabaseModel(CardDatabase *_db, QObject *parent = 0);
	~CardDatabaseModel();
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	CardInfo *getCard(int index) const { return cardList[index]; }
private:
	QList<CardInfo *> cardList;
	CardDatabase *db;
};

class CardDatabaseDisplayModel : public QSortFilterProxyModel {
	Q_OBJECT
private:
	QString cardNameBeginning, cardName, cardText;
	QSet<QString> cardTypes, cardColors;
public:
	CardDatabaseDisplayModel(QObject *parent = 0);
	void setCardNameBeginning(const QString &_beginning) { cardNameBeginning = _beginning; invalidateFilter(); }
	void setCardName(const QString &_cardName) { cardName = _cardName; invalidateFilter(); }
	void setCardText(const QString &_cardText) { cardText = _cardText; invalidateFilter(); }
	void setCardTypes(const QSet<QString> &_cardTypes) { cardTypes = _cardTypes; invalidateFilter(); }
	void setCardColors(const QSet<QString> &_cardColors) { cardColors = _cardColors; invalidateFilter(); }
	void clearSearch();
protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif
