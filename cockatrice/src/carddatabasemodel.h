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
    CardDatabase *getDatabase() const { return db; }
    CardInfo *getCard(int index) const { return cardList[index]; }
private:
    QList<CardInfo *> cardList;
    CardDatabase *db;
private slots:
    void updateCardList();
    void cardAdded(CardInfo *card);
    void cardRemoved(CardInfo *card);
    void cardInfoChanged(CardInfo *card);
};

class CardDatabaseDisplayModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    enum FilterBool { ShowTrue, ShowFalse, ShowAll };
private:
    FilterBool isToken;
    QString cardNameBeginning, cardName, cardText;
    QSet<QString> cardNameSet, cardTypes, cardColors;
public:
    CardDatabaseDisplayModel(QObject *parent = 0);
    void setIsToken(FilterBool _isToken) { isToken = _isToken; invalidate(); }
    void setCardNameBeginning(const QString &_beginning) { cardNameBeginning = _beginning; invalidate(); }
    void setCardName(const QString &_cardName) { cardName = _cardName; invalidate(); }
    void setCardNameSet(const QSet<QString> &_cardNameSet) { cardNameSet = _cardNameSet; invalidate(); }
    void setCardText(const QString &_cardText) { cardText = _cardText; invalidate(); }
    void setCardTypes(const QSet<QString> &_cardTypes) { cardTypes = _cardTypes; invalidate(); }
    void setCardColors(const QSet<QString> &_cardColors) { cardColors = _cardColors; invalidate(); }
    void clearSearch();
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif
