#ifndef DECKLIST_H
#define DECKLIST_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QSet>
#include <QStringList>
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
    explicit AbstractDecklistNode(InnerDecklistNode *_parent = nullptr);
    virtual ~AbstractDecklistNode() = default;
    virtual void setSortMethod(DeckSortMethod method)
    {
        sortMethod = method;
    }
    virtual QString getName() const = 0;
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
private:
    QString name;
    class compareFunctor;

public:
    explicit InnerDecklistNode(QString _name = QString(), InnerDecklistNode *_parent = nullptr)
        : AbstractDecklistNode(_parent), name(std::move(_name))
    {
    }
    explicit InnerDecklistNode(InnerDecklistNode *other, InnerDecklistNode *_parent = nullptr);
    ~InnerDecklistNode() override;
    void setSortMethod(DeckSortMethod method) override;
    QString getName() const override
    {
        return name;
    }
    void setName(const QString &_name)
    {
        name = _name;
    }
    static QString visibleNameFromName(const QString &_name);
    virtual QString getVisibleName() const;
    void clearTree();
    AbstractDecklistNode *findChild(const QString &_name);
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
    explicit AbstractDecklistCardNode(InnerDecklistNode *_parent = nullptr) : AbstractDecklistNode(_parent)
    {
    }
    virtual int getNumber() const = 0;
    virtual void setNumber(int _number) = 0;
    QString getName() const override = 0;
    virtual void setName(const QString &_name) = 0;
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
private:
    QString name;
    int number;

public:
    explicit DecklistCardNode(QString _name = QString(), int _number = 1, InnerDecklistNode *_parent = nullptr)
        : AbstractDecklistCardNode(_parent), name(std::move(_name)), number(_number)
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
};

class DeckList : public QObject
{
    Q_OBJECT
private:
    QString name, comments;
    QString deckHash;
    QMap<QString, SideboardPlan *> sideboardPlans;
    InnerDecklistNode *root;
    void getCardListHelper(InnerDecklistNode *node, QSet<QString> &result) const;
    InnerDecklistNode *getZoneObjFromName(const QString &zoneName);

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

public slots:
    void setName(const QString &_name = QString())
    {
        name = _name;
    }
    void setComments(const QString &_comments = QString())
    {
        comments = _comments;
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
    QList<MoveCard_ToZone> getCurrentSideboardPlan();
    void setCurrentSideboardPlan(const QList<MoveCard_ToZone> &plan);
    const QMap<QString, SideboardPlan *> &getSideboardPlans() const
    {
        return sideboardPlans;
    }

    bool readElement(QXmlStreamReader *xml);
    void write(QXmlStreamWriter *xml);
    bool loadFromXml(QXmlStreamReader *xml);
    bool loadFromString_Native(const QString &nativeString);
    QString writeToString_Native();
    bool loadFromFile_Native(QIODevice *device);
    bool saveToFile_Native(QIODevice *device);
    bool loadFromStream_Plain(QTextStream &stream);
    bool loadFromFile_Plain(QIODevice *device);
    bool saveToStream_Plain(QTextStream &stream, bool prefixSideboardCards, bool slashTappedOutSplitCards);
    bool saveToFile_Plain(QIODevice *device, bool prefixSideboardCards = true, bool slashTappedOutSplitCards = false);
    QString writeToString_Plain(bool prefixSideboardCards = true, bool slashTappedOutSplitCards = false);

    void cleanList();
    bool isEmpty() const
    {
        return root->isEmpty() && name.isEmpty() && comments.isEmpty() && sideboardPlans.isEmpty();
    }
    QStringList getCardList() const;

    int getSideboardSize() const;

    QString getDeckHash() const
    {
        return deckHash;
    }
    void updateDeckHash();

    InnerDecklistNode *getRoot() const
    {
        return root;
    }
    DecklistCardNode *addCard(const QString &cardName, const QString &zoneName);
    bool deleteNode(AbstractDecklistNode *node, InnerDecklistNode *rootNode = nullptr);

    /**
     * Calls a given function object for each card in the deck. It must
     * take a InnerDecklistNode* as its first argument and a
     * DecklistCardNode* as its second.
     */
    template <typename Callback> void forEachCard(Callback &callback) const
    {
        // Support for this is only possible if the internal structure
        // doesn't get more complicated.
        for (int i = 0; i < root->size(); i++) {
            const InnerDecklistNode *node = dynamic_cast<InnerDecklistNode *>(root->at(i));
            for (int j = 0; j < node->size(); j++) {
                const DecklistCardNode *card = dynamic_cast<DecklistCardNode *>(node->at(j));
                callback(node, card);
            }
        }
    }
};

#endif
