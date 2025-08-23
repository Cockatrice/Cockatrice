#ifndef DECKLIST_H
#define DECKLIST_H

#include "card_ref.h"

#include <QMap>
#include <QVector>

// Required on Mac. Forward declaration doesn't work. Don't ask why.
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>
#include <common/pb/move_card_to_zone.pb.h>

class CardDatabase;
class QIODevice;
class QTextStream;

class InnerDecklistNode;

#define DECK_ZONE_MAIN "main"
#define DECK_ZONE_SIDE "side"
#define DECK_ZONE_TOKENS "tokens"

class SideboardPlan
{
private:
    QString name;
    QList<MoveCard_ToZone> moveList;

public:
    explicit SideboardPlan(const QString &_name = QString(),
                           const QList<MoveCard_ToZone> &_moveList = QList<MoveCard_ToZone>());
    bool readElement(QXmlStreamReader *xml);
    void write(QXmlStreamWriter *xml);

    QString getName() const
    {
        return name;
    }
    const QList<MoveCard_ToZone> &getMoveList() const
    {
        return moveList;
    }
    void setMoveList(const QList<MoveCard_ToZone> &_moveList);
};

enum DeckSortMethod
{
    ByNumber,
    ByName,
    Default
};

class AbstractDecklistNode
{
protected:
    InnerDecklistNode *parent;
    DeckSortMethod sortMethod;

public:
    explicit AbstractDecklistNode(InnerDecklistNode *_parent = nullptr, int position = -1);
    virtual ~AbstractDecklistNode() = default;
    virtual void setSortMethod(DeckSortMethod method)
    {
        sortMethod = method;
    }
    virtual QString getName() const = 0;
    virtual QString getCardProviderId() const = 0;
    virtual QString getCardSetShortName() const = 0;
    virtual QString getCardCollectorNumber() const = 0;
    [[nodiscard]] virtual bool isDeckHeader() const = 0;
    InnerDecklistNode *getParent() const
    {
        return parent;
    }
    int depth() const;
    virtual int height() const = 0;
    virtual bool compare(AbstractDecklistNode *other) const = 0;

    virtual bool readElement(QXmlStreamReader *xml) = 0;
    virtual void writeElement(QXmlStreamWriter *xml) = 0;
};

class InnerDecklistNode : public AbstractDecklistNode, public QList<AbstractDecklistNode *>
{
    QString name;

public:
    explicit InnerDecklistNode(QString _name = QString(), InnerDecklistNode *_parent = nullptr, int position = -1)
        : AbstractDecklistNode(_parent, position), name(std::move(_name))
    {
    }
    explicit InnerDecklistNode(InnerDecklistNode *other, InnerDecklistNode *_parent = nullptr);
    ~InnerDecklistNode() override;
    void setSortMethod(DeckSortMethod method) override;
    [[nodiscard]] QString getName() const override
    {
        return name;
    }
    void setName(const QString &_name)
    {
        name = _name;
    }
    static QString visibleNameFromName(const QString &_name);
    [[nodiscard]] virtual QString getVisibleName() const;
    [[nodiscard]] QString getCardProviderId() const override
    {
        return "";
    }
    [[nodiscard]] QString getCardSetShortName() const override
    {
        return "";
    }
    [[nodiscard]] QString getCardCollectorNumber() const override
    {
        return "";
    }
    [[nodiscard]] bool isDeckHeader() const override
    {
        return true;
    }

    void clearTree();
    AbstractDecklistNode *findChild(const QString &_name);
    AbstractDecklistNode *findCardChildByNameProviderIdAndNumber(const QString &_name,
                                                                 const QString &_providerId = "",
                                                                 const QString &_cardNumber = "");
    int height() const override;
    int recursiveCount(bool countTotalCards = false) const;
    bool compare(AbstractDecklistNode *other) const override;
    bool compareNumber(AbstractDecklistNode *other) const;
    bool compareName(AbstractDecklistNode *other) const;
    QVector<QPair<int, int>> sort(Qt::SortOrder order = Qt::AscendingOrder);

    bool readElement(QXmlStreamReader *xml) override;
    void writeElement(QXmlStreamWriter *xml) override;
};

class AbstractDecklistCardNode : public AbstractDecklistNode
{
public:
    explicit AbstractDecklistCardNode(InnerDecklistNode *_parent = nullptr, int position = -1)
        : AbstractDecklistNode(_parent, position)
    {
    }
    virtual int getNumber() const = 0;
    virtual void setNumber(int _number) = 0;
    QString getName() const override = 0;
    virtual void setName(const QString &_name) = 0;
    virtual QString getCardProviderId() const override = 0;
    virtual void setCardProviderId(const QString &_cardProviderId) = 0;
    virtual QString getCardSetShortName() const override = 0;
    virtual void setCardSetShortName(const QString &_cardSetShortName) = 0;
    virtual QString getCardCollectorNumber() const override = 0;
    virtual void setCardCollectorNumber(const QString &_cardSetNumber) = 0;
    int height() const override
    {
        return 0;
    }
    bool compare(AbstractDecklistNode *other) const override;
    bool compareNumber(AbstractDecklistNode *other) const;
    bool compareName(AbstractDecklistNode *other) const;

    bool readElement(QXmlStreamReader *xml) override;
    void writeElement(QXmlStreamWriter *xml) override;
};

class DecklistCardNode : public AbstractDecklistCardNode
{
    QString name;
    int number;
    QString cardSetShortName;
    QString cardSetNumber;
    QString cardProviderId;

public:
    explicit DecklistCardNode(QString _name = QString(),
                              int _number = 1,
                              InnerDecklistNode *_parent = nullptr,
                              int position = -1,
                              QString _cardSetShortName = QString(),
                              QString _cardSetNumber = QString(),
                              QString _cardProviderId = QString())
        : AbstractDecklistCardNode(_parent, position), name(std::move(_name)), number(_number),
          cardSetShortName(std::move(_cardSetShortName)), cardSetNumber(std::move(_cardSetNumber)),
          cardProviderId(std::move(_cardProviderId))
    {
    }
    explicit DecklistCardNode(DecklistCardNode *other, InnerDecklistNode *_parent);
    int getNumber() const override
    {
        return number;
    }
    void setNumber(int _number) override
    {
        number = _number;
    }
    QString getName() const override
    {
        return name;
    }
    void setName(const QString &_name) override
    {
        name = _name;
    }
    QString getCardProviderId() const override
    {
        return cardProviderId;
    }
    void setCardProviderId(const QString &_providerId) override
    {
        cardProviderId = _providerId;
    }

    QString getCardSetShortName() const override
    {
        return cardSetShortName;
    }
    void setCardSetShortName(const QString &_cardSetShortName) override
    {
        cardSetShortName = _cardSetShortName;
    }
    QString getCardCollectorNumber() const override
    {
        return cardSetNumber;
    }
    void setCardCollectorNumber(const QString &_cardSetNumber) override
    {
        cardSetNumber = _cardSetNumber;
    }
    [[nodiscard]] bool isDeckHeader() const override
    {
        return false;
    }
    CardRef toCardRef() const
    {
        return {name, cardProviderId};
    }
};

class DeckList : public QObject
{
    Q_OBJECT
private:
    QString name, comments;
    CardRef bannerCard;
    QString lastLoadedTimestamp;
    QStringList tags;
    QMap<QString, SideboardPlan *> sideboardPlans;
    InnerDecklistNode *root;
    static void getCardListHelper(InnerDecklistNode *node, QSet<QString> &result);
    static void getCardRefListHelper(InnerDecklistNode *item, QList<CardRef> &result);
    InnerDecklistNode *getZoneObjFromName(const QString &zoneName);

    /**
     * Empty string indicates invalidated cache.
     */
    mutable QString cachedDeckHash;

protected:
    virtual QString getCardZoneFromName(const QString /*cardName*/, QString currentZoneName)
    {
        return currentZoneName;
    };
    virtual QString getCompleteCardName(const QString &cardName) const
    {
        return cardName;
    };

signals:
    void deckHashChanged();
    void deckTagsChanged();

public slots:
    void setName(const QString &_name = QString())
    {
        name = _name;
    }
    void setComments(const QString &_comments = QString())
    {
        comments = _comments;
    }
    void setTags(const QStringList &_tags = QStringList())
    {
        tags = _tags;
        emit deckTagsChanged();
    }
    void addTag(const QString &_tag)
    {
        tags.append(_tag);
        emit deckTagsChanged();
    }
    void clearTags()
    {
        tags.clear();
        emit deckTagsChanged();
    }
    void setBannerCard(const CardRef &_bannerCard = {})
    {
        bannerCard = _bannerCard;
    }
    void setLastLoadedTimestamp(const QString &_lastLoadedTimestamp = QString())
    {
        lastLoadedTimestamp = _lastLoadedTimestamp;
    }

public:
    explicit DeckList();
    DeckList(const DeckList &other);
    explicit DeckList(const QString &nativeString);
    ~DeckList() override;
    QString getName() const
    {
        return name;
    }
    QString getComments() const
    {
        return comments;
    }
    QStringList getTags() const
    {
        return tags;
    }
    CardRef getBannerCard() const
    {
        return bannerCard;
    }
    QString getLastLoadedTimestamp() const
    {
        return lastLoadedTimestamp;
    }
    QList<MoveCard_ToZone> getCurrentSideboardPlan();
    void setCurrentSideboardPlan(const QList<MoveCard_ToZone> &plan);
    const QMap<QString, SideboardPlan *> &getSideboardPlans() const
    {
        return sideboardPlans;
    }

    bool readElement(QXmlStreamReader *xml);
    void write(QXmlStreamWriter *xml) const;
    bool loadFromXml(QXmlStreamReader *xml);
    bool loadFromString_Native(const QString &nativeString);
    QString writeToString_Native() const;
    bool loadFromFile_Native(QIODevice *device);
    bool saveToFile_Native(QIODevice *device);
    bool loadFromStream_Plain(QTextStream &stream, bool preserveMetadata);
    bool loadFromFile_Plain(QIODevice *device);
    bool saveToStream_Plain(QTextStream &stream, bool prefixSideboardCards, bool slashTappedOutSplitCards);
    bool saveToFile_Plain(QIODevice *device, bool prefixSideboardCards = true, bool slashTappedOutSplitCards = false);
    QString writeToString_Plain(bool prefixSideboardCards = true, bool slashTappedOutSplitCards = false);

    void cleanList(bool preserveMetadata = false);
    bool isEmpty() const
    {
        return root->isEmpty() && name.isEmpty() && comments.isEmpty() && sideboardPlans.isEmpty();
    }
    QStringList getCardList() const;
    QList<CardRef> getCardRefList() const;

    int getSideboardSize() const;

    InnerDecklistNode *getRoot() const
    {
        return root;
    }
    DecklistCardNode *addCard(const QString &cardName,
                              const QString &zoneName,
                              int position,
                              const QString &cardSetName = QString(),
                              const QString &cardSetCollectorNumber = QString(),
                              const QString &cardProviderId = QString());
    bool deleteNode(AbstractDecklistNode *node, InnerDecklistNode *rootNode = nullptr);

    QString getDeckHash() const;
    void refreshDeckHash();

    void forEachCard(const std::function<void(InnerDecklistNode *, DecklistCardNode *)> &func);
};

#endif