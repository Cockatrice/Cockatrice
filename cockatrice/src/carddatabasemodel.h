#ifndef CARDDATABASEMODEL_H
#define CARDDATABASEMODEL_H

#include "carddatabase.h"
#include <QAbstractListModel>
#include <QList>
#include <QSet>
#include <QSortFilterProxyModel>

class FilterTree;

class CardDatabaseModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Columns
    {
        NameColumn,
        SetListColumn,
        ManaCostColumn,
        PTColumn,
        CardTypeColumn,
        ColorColumn
    };
    enum Role
    {
        SortRole = Qt::UserRole
    };
    CardDatabaseModel(CardDatabase *_db, bool _showOnlyCardsFromEnabledSets, QObject *parent = nullptr);
    ~CardDatabaseModel() override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    CardDatabase *getDatabase() const
    {
        return db;
    }
    CardInfoPtr getCard(int index) const
    {
        return cardList[index];
    }

private:
    QList<CardInfoPtr> cardList;
    CardDatabase *db;
    bool showOnlyCardsFromEnabledSets;

    inline bool checkCardHasAtLeastOneEnabledSet(CardInfoPtr card);
private slots:
    void cardAdded(CardInfoPtr card);
    void cardRemoved(CardInfoPtr card);
    void cardInfoChanged(CardInfoPtr card);
    void cardDatabaseEnabledSetsChanged();
};

class CardDatabaseDisplayModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum FilterBool
    {
        ShowTrue,
        ShowFalse,
        ShowAll
    };

private:
    FilterBool isToken;
    QString cardNameBeginning, cardName, cardText;
    QString searchTerm;
    QSet<QString> cardNameSet, cardTypes, cardColors;
    FilterTree *filterTree;
    int loadedRowCount;

    /** The translation table that will be used for sanitizeCardName. */
    static QMap<wchar_t, wchar_t> characterTranslation;

public:
    explicit CardDatabaseDisplayModel(QObject *parent = nullptr);
    void setFilterTree(FilterTree *filterTree);
    void setIsToken(FilterBool _isToken)
    {
        isToken = _isToken;
        invalidate();
    }
    void setCardNameBeginning(const QString &_beginning)
    {
        cardNameBeginning = _beginning;
        invalidate();
    }
    void setCardName(const QString &_cardName)
    {
        cardName = sanitizeCardName(_cardName, characterTranslation);
        invalidate();
    }
    void setCardNameSet(const QSet<QString> &_cardNameSet)
    {
        cardNameSet = _cardNameSet;
        invalidate();
    }
    void setSearchTerm(const QString &_searchTerm)
    {
        searchTerm = _searchTerm;
    }
    void setCardText(const QString &_cardText)
    {
        cardText = _cardText;
        invalidate();
    }
    void setCardTypes(const QSet<QString> &_cardTypes)
    {
        cardTypes = _cardTypes;
        invalidate();
    }
    void setCardColors(const QSet<QString> &_cardColors)
    {
        cardColors = _cardColors;
        invalidate();
    }
    void clearFilterAll();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    static int lessThanNumerically(const QString &left, const QString &right);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool rowMatchesCardName(CardInfoPtr info) const;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;
private slots:
    void filterTreeChanged();
    /** Will translate all undesirable characters in DIRTYNAME according to the TABLE. */
    const QString sanitizeCardName(const QString &dirtyName, const QMap<wchar_t, wchar_t> &table);
};

class TokenDisplayModel : public CardDatabaseDisplayModel
{
    Q_OBJECT
public:
    explicit TokenDisplayModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

#endif
