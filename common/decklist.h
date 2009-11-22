#ifndef DECKLIST_H
#define DECKLIST_H

#include <QList>
#include <QVector>
#include <QPair>
#include <QObject>

class CardDatabase;
class QIODevice;
class QXmlStreamReader;
class QXmlStreamWriter;

class InnerDecklistNode;

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
	virtual ~InnerDecklistNode();
	QString getName() const { return name; }
	void setName(const QString &_name) { name = _name; }
	static QString visibleNameFromName(const QString &_name);
	virtual QString getVisibleName() const;
	void clearTree();
	AbstractDecklistNode *findChild(const QString &name);
	int height() const;
	int recursiveCount(bool countTotalCards = false) const;
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
	int height() const { return 0; }
	bool compare(AbstractDecklistNode *other) const;
	
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
};

class DecklistCardNode : public AbstractDecklistCardNode {
private:
	QString name;
	int number;
public:
	DecklistCardNode(const QString &_name = QString(), int _number = 1, InnerDecklistNode *_parent = 0) : AbstractDecklistCardNode(_parent), name(_name), number(_number) { }
	int getNumber() const { return number; }
	void setNumber(int _number) { number = _number; }
	QString getName() const { return name; }
	void setName(const QString &_name) { name = _name; }
};

class DeckList : public QObject {
	Q_OBJECT
public:
	enum FileFormat { PlainTextFormat, CockatriceFormat };
private:
	QString name, comments;
	QString lastFileName;
	FileFormat lastFileFormat;
	InnerDecklistNode *root;
	InnerDecklistNode *currentZone;
	QString currentElementText;
signals:
	void deckLoaded();
public slots:
	void setName(const QString &_name = QString()) { name = _name; }
	void setComments(const QString &_comments = QString()) { comments = _comments; }
public:
	static const QStringList fileNameFilters;
	DeckList(QObject *parent = 0);
	~DeckList();
	QString getName() const { return name; }
	QString getComments() const { return comments; }
	QString getLastFileName() const { return lastFileName; }
	FileFormat getLastFileFormat() const { return lastFileFormat; }

	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
	bool loadFromXml(QXmlStreamReader *xml);
	
	bool loadFromFile_Native(QIODevice *device);
	bool saveToFile_Native(QIODevice *device);
	bool loadFromFile_Plain(QIODevice *device);
	bool saveToFile_Plain(QIODevice *device);
	bool loadFromFile(const QString &fileName, FileFormat fmt);
	bool saveToFile(const QString &fileName, FileFormat fmt);
	static FileFormat getFormatFromNameFilter(const QString &selectedNameFilter);

	void cleanList();

	InnerDecklistNode *getRoot() const { return root; }
	DecklistCardNode *addCard(const QString &cardName, const QString &zoneName);
	bool deleteNode(AbstractDecklistNode *node, InnerDecklistNode *rootNode = 0);
};

#endif
