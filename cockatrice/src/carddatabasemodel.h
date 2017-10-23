#ifndef CARDDATABASEMODEL_H
#define CARDDATABASEMODEL_H

#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QList>
#include <QSet>
#include "carddatabase.h"

class FilterTree;

class CardDatabaseModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Columns { NameColumn, SetListColumn, ManaCostColumn, PTColumn, CardTypeColumn, ColorColumn };
    enum Role { SortRole=Qt::UserRole };
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

    inline bool checkCardHasAtLeastOneEnabledSet(CardInfo *card);
private slots:
    void cardAdded(CardInfo *card);
    void cardRemoved(CardInfo *card);
    void cardInfoChanged(CardInfo *card);
    void cardDatabaseEnabledSetsChanged();
};

class CardDatabaseDisplayModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    enum FilterBool { ShowTrue, ShowFalse, ShowAll };
private:
    FilterBool isToken;
    QString cardNameBeginning, cardName, cardText;
    QString searchTerm;
    QSet<QString> cardNameSet, cardTypes, cardColors;
    FilterTree *filterTree;
    int loadedRowCount;
public:
    CardDatabaseDisplayModel(QObject *parent = 0);
    void setFilterTree(FilterTree *filterTree);
    void setIsToken(FilterBool _isToken) { isToken = _isToken; invalidate(); }
    void setCardNameBeginning(const QString &_beginning) { cardNameBeginning = _beginning; invalidate(); }
    void setCardName(const QString &_cardName) { cardName = _cardName; invalidate(); }
    void setCardNameSet(const QSet<QString> &_cardNameSet) { cardNameSet = _cardNameSet; invalidate(); }
    void setSearchTerm(const QString &_searchTerm) { searchTerm = _searchTerm; }
    void setCardText(const QString &_cardText) { cardText = _cardText; invalidate(); }
    void setCardTypes(const QSet<QString> &_cardTypes) { cardTypes = _cardTypes; invalidate(); }
    void setCardColors(const QSet<QString> &_cardColors) { cardColors = _cardColors; invalidate(); }
    void clearFilterAll();
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool rowMatchesCardName(CardInfo const *info) const;
    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);
private slots:
    void filterTreeChanged();
};

class TokenDisplayModel : public CardDatabaseDisplayModel {
    Q_OBJECT
public:
    TokenDisplayModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif
