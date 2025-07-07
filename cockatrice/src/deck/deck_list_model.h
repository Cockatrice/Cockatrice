#ifndef DECKLISTMODEL_H
#define DECKLISTMODEL_H

#include "../game/cards/card_info.h"
#include "decklist.h"

#include <QAbstractItemModel>
#include <QList>

class DeckLoader;
class CardDatabase;
class QPrinter;
class QTextCursor;

enum DeckListModelGroupCriteria
{
    MAIN_TYPE,
    MANA_COST,
    COLOR
};

class DecklistModelCardNode : public AbstractDecklistCardNode
{
private:
    DecklistCardNode *dataNode;

public:
    DecklistModelCardNode(DecklistCardNode *_dataNode, InnerDecklistNode *_parent, int position = -1)
        : AbstractDecklistCardNode(_parent, position), dataNode(_dataNode)
    {
    }
    int getNumber() const override
    {
        return dataNode->getNumber();
    }
    void setNumber(int _number) override
    {
        dataNode->setNumber(_number);
    }
    QString getName() const override
    {
        return dataNode->getName();
    }
    void setName(const QString &_name) override
    {
        dataNode->setName(_name);
    }
    QString getCardProviderId() const override
    {
        return dataNode->getCardProviderId();
    }
    void setCardProviderId(const QString &_cardProviderId) override
    {
        dataNode->setCardProviderId(_cardProviderId);
    }
    QString getCardSetShortName() const override
    {
        return dataNode->getCardSetShortName();
    }
    void setCardSetShortName(const QString &_cardSetShortName) override
    {
        dataNode->setCardSetShortName(_cardSetShortName);
    }
    QString getCardCollectorNumber() const override
    {
        return dataNode->getCardCollectorNumber();
    }
    void setCardCollectorNumber(const QString &_cardSetNumber) override
    {
        dataNode->setCardCollectorNumber(_cardSetNumber);
    }
    DecklistCardNode *getDataNode() const
    {
        return dataNode;
    }
    [[nodiscard]] bool isDeckHeader() const override
    {
        return false;
    }
};

class DeckListModel : public QAbstractItemModel
{
    Q_OBJECT
private slots:
    void rebuildTree();
public slots:
    void printDeckList(QPrinter *printer);
signals:
    void deckHashChanged();

public:
    explicit DeckListModel(QObject *parent = nullptr);
    ~DeckListModel() override;
    QModelIndex getRoot() const
    {
        return nodeToIndex(root);
    };
    QString getGroupCriteriaForCard(CardInfoPtr info) const;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex & /*parent*/ = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    QModelIndex findCard(const QString &cardName,
                         const QString &zoneName,
                         const QString &providerId = "",
                         const QString &cardNumber = "") const;
    QModelIndex addPreferredPrintingCard(const QString &cardName, const QString &zoneName, bool abAddAnyway);
    QModelIndex addCard(const ::QString &cardName,
                        const PrintingInfo &cardInfoSet,
                        const QString &zoneName,
                        bool abAddAnyway = false);
    int findSortedInsertRow(InnerDecklistNode *parent, CardInfoPtr cardInfo) const;
    void sort(int column, Qt::SortOrder order) override;
    void cleanList();
    DeckLoader *getDeckList() const
    {
        return deckList;
    }
    void setDeckList(DeckLoader *_deck);
    QList<CardInfoPtr> getCardsAsCardInfoPtrs() const;
    QList<CardInfoPtr> getCardsAsCardInfoPtrsForZone(QString zoneName) const;
    QList<QString> *getZones() const;
    void setActiveGroupCriteria(DeckListModelGroupCriteria newCriteria);

private:
    DeckLoader *deckList;
    InnerDecklistNode *root;
    DeckListModelGroupCriteria activeGroupCriteria = DeckListModelGroupCriteria::MAIN_TYPE;
    int lastKnownColumn;
    Qt::SortOrder lastKnownOrder;
    InnerDecklistNode *createNodeIfNeeded(const QString &name, InnerDecklistNode *parent);
    QModelIndex nodeToIndex(AbstractDecklistNode *node) const;
    DecklistModelCardNode *findCardNode(const QString &cardName,
                                        const QString &zoneName,
                                        const QString &providerId = "",
                                        const QString &cardNumber = "") const;
    void emitRecursiveUpdates(const QModelIndex &index);
    void sortHelper(InnerDecklistNode *node, Qt::SortOrder order);

    void printDeckListNode(QTextCursor *cursor, InnerDecklistNode *node);

    template <typename T> T getNode(const QModelIndex &index) const
    {
        if (!index.isValid())
            return dynamic_cast<T>(root);
        return dynamic_cast<T>(static_cast<AbstractDecklistNode *>(index.internalPointer()));
    }
};

#endif
