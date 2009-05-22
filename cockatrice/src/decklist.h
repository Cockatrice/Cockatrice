#ifndef DECKLIST_H
#define DECKLIST_H

#include <QList>
#include <QObject>

class CardDatabase;
class QIODevice;

class DecklistRow {
private:
	int number;
	QString card;
	bool sideboard;
public:
	DecklistRow(int _number, const QString &_card, bool _sideboard) : number(_number), card(_card), sideboard(_sideboard) { }
	int getNumber() const { return number; }
	QString getCard() const { return card; }
	bool isSideboard() const { return sideboard; }
};

class DeckList : public QObject, public QList<DecklistRow *> {
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
public slots:
	void setName(const QString &_name) { name = _name; }
	void setComments(const QString &_comments) { comments = _comments; }
public:
	DeckList(CardDatabase *_db);
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
};

#endif
