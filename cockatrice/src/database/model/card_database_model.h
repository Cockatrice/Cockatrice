/**
 * @file card_database_model.h
 * @ingroup CardDatabaseModels
 * @brief The CardDatabaseModel maps the cardList contained in the CardDatabase as a QAbstractListModel.
 */

#ifndef CARDDATABASEMODEL_H
#define CARDDATABASEMODEL_H

#include "../card_database.h"

#include <QAbstractListModel>
#include <QList>
#include <QSet>

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
    QSet<CardInfoPtr> cardListSet; // Supports faster lookups in cardDatabaseEnabledSetsChanged()
    CardDatabase *db;
    bool showOnlyCardsFromEnabledSets;

    inline bool checkCardHasAtLeastOneEnabledSet(CardInfoPtr card);
private slots:
    void cardAdded(CardInfoPtr card);
    void cardRemoved(CardInfoPtr card);
    void cardInfoChanged(CardInfoPtr card);
    void cardDatabaseEnabledSetsChanged();
};

#endif
