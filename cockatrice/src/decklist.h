#ifndef DECKLIST_H
#define DECKLIST_H

#include <QList>
#include <QObject>

class CardDatabase;
class QIODevice;
class QProgressDialog;

class InnerDecklistNode;

class AbstractDecklistNode {
protected:
	InnerDecklistNode *parent;
public:
	AbstractDecklistNode(InnerDecklistNode *_parent = 0);
	virtual bool hasChildren() const = 0;
	virtual QString getName() const = 0;
	const InnerDecklistNode *getParent() const { return parent; }
};

class InnerDecklistNode : public AbstractDecklistNode, public QList<AbstractDecklistNode *> {
private:
	QString name;
public:
	InnerDecklistNode(const QString &_name = QString(), InnerDecklistNode *_parent = 0) : AbstractDecklistNode(_parent), name(_name) { }
	~InnerDecklistNode();
	bool hasChildren() const { return true; }
	QString getName() const { return name; }
	void setName(const QString &_name) { name = _name; }
	virtual QString getVisibleName() const;
	void clearTree();
	int recursiveCount() const;
};

class DecklistCardNode : public AbstractDecklistNode {
private:
	QString name;
	int number;
public:
	DecklistCardNode(const QString &_name = QString(), int _number = 1, InnerDecklistNode *_parent = 0) : AbstractDecklistNode(_parent), name(_name), number(_number) { }
	bool hasChildren() const { return false; }
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
	static const QStringList fileNameFilters;
	void cacheCardPictures(QWidget *parent = 0);
	CardDatabase *db;
	QString name, comments;
	QString lastFileName;
	FileFormat lastFileFormat;
	InnerDecklistNode *root;
	void cacheCardPicturesHelper(InnerDecklistNode *item, QProgressDialog *progress);
signals:
	void deckLoaded();
public slots:
	void setName(const QString &_name = QString()) { name = _name; }
	void setComments(const QString &_comments = QString()) { comments = _comments; }
public:
	DeckList(CardDatabase *_db, QObject *parent = 0);
	~DeckList();
	QString getName() const { return name; }
	QString getComments() const { return comments; }
	QString getLastFileName() const { return lastFileName; }
	FileFormat getLastFileFormat() const { return lastFileFormat; }

	bool loadFromFile_Native(QIODevice *device);
	bool saveToFile_Native(QIODevice *device);
	bool loadFromFile_Plain(QIODevice *device);
	bool saveToFile_Plain(QIODevice *device);
	bool loadFromFile(const QString &fileName, FileFormat fmt, QWidget *parent = 0);
	bool saveToFile(const QString &fileName, FileFormat fmt);
	bool loadDialog(QWidget *parent = 0);
	bool saveDialog(QWidget *parent = 0);

	void cleanList();
	void initZones();

	InnerDecklistNode *getRoot() const { return root; }
};

#endif
