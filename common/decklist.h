#ifndef DECKLIST_H
#define DECKLIST_H

#include <QList>
#include <QVector>
#include <QPair>
#include <QObject>
#include <QStringList>
#include <QSet>
#include "serializable_item.h"

class CardDatabase;
class QIODevice;
class QTextStream;
class QXmlStreamReader;
class QXmlStreamWriter;

class InnerDecklistNode;

class MoveCardToZone : public SerializableItem_Map {
public:
	MoveCardToZone(const QString &_cardName = QString(), const QString &_startZone = QString(), const QString &_targetZone = QString());
	MoveCardToZone(MoveCardToZone *other);
	static SerializableItem *newItem() { return new MoveCardToZone; }
	QString getCardName() const { return static_cast<SerializableItem_String *>(itemMap.value("card_name"))->getData(); }
	QString getStartZone() const { return static_cast<SerializableItem_String *>(itemMap.value("start_zone"))->getData(); }
	QString getTargetZone() const { return static_cast<SerializableItem_String *>(itemMap.value("target_zone"))->getData(); }
};

class SideboardPlan : public SerializableItem_Map {
public:
	SideboardPlan(const QString &_name = QString(), const QList<MoveCardToZone *> &_moveList = QList<MoveCardToZone *>());
	static SerializableItem *newItem() { return new SideboardPlan; }
	QString getName() const { return static_cast<SerializableItem_String *>(itemMap.value("name"))->getData(); }
	QList<MoveCardToZone *> getMoveList() const { return typecastItemList<MoveCardToZone *>(); }
	void setMoveList(const QList<MoveCardToZone *> &_moveList);
};

class AbstractDecklistNode {
protected:
	InnerDecklistNode *parent;
	AbstractDecklistNode *currentItem;
public:
	AbstractDecklistNode(InnerDecklistNode *_parent = 0);
	virtual ~AbstractDecklistNode() { }
	virtual QString getName() const = 0;
	InnerDecklistNode *getParent() const { return parent; }
	int depth() const;
	virtual int height() const = 0;
	virtual bool compare(AbstractDecklistNode *other) const = 0;
	
	virtual bool readElement(QXmlStreamReader *xml) = 0;
	virtual void writeElement(QXmlStreamWriter *xml) = 0;
};

class InnerDecklistNode : public AbstractDecklistNode, public QList<AbstractDecklistNode *> {
private:
	QString name;
	class compareFunctor;
public:
	InnerDecklistNode(const QString &_name = QString(), InnerDecklistNode *_parent = 0) : AbstractDecklistNode(_parent), name(_name) { }
	InnerDecklistNode(InnerDecklistNode *other, InnerDecklistNode *_parent = 0);
	virtual ~InnerDecklistNode();
	QString getName() const { return name; }
	void setName(const QString &_name) { name = _name; }
	static QString visibleNameFromName(const QString &_name);
	virtual QString getVisibleName() const;
	void clearTree();
	AbstractDecklistNode *findChild(const QString &name);
	int height() const;
	int recursiveCount(bool countTotalCards = false) const;
        float recursivePrice(bool countTotalCards = false) const;
	bool compare(AbstractDecklistNode *other) const;
	QVector<QPair<int, int> > sort(Qt::SortOrder order = Qt::AscendingOrder);
	
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
};

class AbstractDecklistCardNode : public AbstractDecklistNode {
public:
	AbstractDecklistCardNode(InnerDecklistNode *_parent = 0) : AbstractDecklistNode(_parent) { }
	virtual int getNumber() const = 0;
	virtual void setNumber(int _number) = 0;
	virtual QString getName() const = 0;
	virtual void setName(const QString &_name) = 0;
        virtual float getPrice() const = 0;
        virtual void setPrice(float _price) = 0;
        float getTotalPrice() const { return getNumber() * getPrice(); }
	int height() const { return 0; }
	bool compare(AbstractDecklistNode *other) const;
	
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
};

class DecklistCardNode : public AbstractDecklistCardNode {
private:
	QString name;
	int number;
        float price;
public:
        DecklistCardNode(const QString &_name = QString(), int _number = 1, float _price = 0, InnerDecklistNode *_parent = 0) : AbstractDecklistCardNode(_parent), name(_name), number(_number), price(_price) { }
        DecklistCardNode(const QString &_name = QString(), int _number = 1, InnerDecklistNode *_parent = 0) : AbstractDecklistCardNode(_parent), name(_name), number(_number), price(0) { }
	DecklistCardNode(DecklistCardNode *other, InnerDecklistNode *_parent);
	int getNumber() const { return number; }
	void setNumber(int _number) { number = _number; }
	QString getName() const { return name; }
	void setName(const QString &_name) { name = _name; }
        float getPrice() const { return price; }
        void setPrice(const float _price) { price = _price; }
    };

class DeckList : public SerializableItem {
	Q_OBJECT
public:
	enum FileFormat { PlainTextFormat, CockatriceFormat };
private:
	QString name, comments;
	QString lastFileName;
	FileFormat lastFileFormat;
	QMap<QString, SideboardPlan *> sideboardPlans;
	InnerDecklistNode *root;
	InnerDecklistNode *currentZone;
	SideboardPlan *currentSideboardPlan;
	QString currentElementText;
	void getCardListHelper(InnerDecklistNode *node, QSet<QString> &result) const;
signals:
	void deckLoaded();
public slots:
	void setName(const QString &_name = QString()) { name = _name; }
	void setComments(const QString &_comments = QString()) { comments = _comments; }
public:
	static const QStringList fileNameFilters;
	DeckList();
	DeckList(DeckList *other);
	~DeckList();
	QString getName() const { return name; }
	QString getComments() const { return comments; }
	QString getLastFileName() const { return lastFileName; }
	FileFormat getLastFileFormat() const { return lastFileFormat; }
	QList<MoveCardToZone *> getCurrentSideboardPlan();
	void setCurrentSideboardPlan(const QList<MoveCardToZone *> &plan);
	const QMap<QString, SideboardPlan *> &getSideboardPlans() const { return sideboardPlans; }

	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
	void loadFromXml(QXmlStreamReader *xml);
	
	bool loadFromFile_Native(QIODevice *device);
	bool saveToFile_Native(QIODevice *device);
	bool loadFromStream_Plain(QTextStream &stream);
	bool loadFromFile_Plain(QIODevice *device);
	bool saveToStream_Plain(QTextStream &stream);
	bool saveToFile_Plain(QIODevice *device);
	bool loadFromFile(const QString &fileName, FileFormat fmt);
	bool saveToFile(const QString &fileName, FileFormat fmt);
	static FileFormat getFormatFromNameFilter(const QString &selectedNameFilter);

	void cleanList();
	bool isEmpty() const { return root->isEmpty() && name.isEmpty() && comments.isEmpty() && sideboardPlans.isEmpty(); }
	QStringList getCardList() const;

	InnerDecklistNode *getRoot() const { return root; }
	DecklistCardNode *addCard(const QString &cardName, const QString &zoneName);
	bool deleteNode(AbstractDecklistNode *node, InnerDecklistNode *rootNode = 0);
};

#endif
