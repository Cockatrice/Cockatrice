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
public:
	DecklistRow(int _number = 1, const QString &_card = QString()) : number(_number), card(_card) { }
	int getNumber() const { return number; }
	void setNumber(int _number) { number = _number; }
	QString getCard() const { return card; }
	void setCard(const QString &_card) { card = _card; }
};

class DecklistZone : public QList<DecklistRow *> {
private:
	QString name;
public:
	DecklistZone(const QString &_name) : name(_name) { }
	QString getName() const { return name; }
	QString getVisibleName() const;
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
	QList<DecklistZone *> zones;
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

	int zoneCount() const { return zones.size(); }
	DecklistZone *getZoneByIndex(int index) const { return zones[index]; }
};

#endif
