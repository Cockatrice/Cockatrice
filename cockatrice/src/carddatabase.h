#ifndef CARDDATABASE_H
#define CARDDATABASE_H

#include <QHash>
#include <QPixmap>
#include <QMap>
#include <QDataStream>
#include <QList>

class CardDatabase;
class CardInfo;

class CardSet : public QList<CardInfo *> {
private:
	QString shortName, longName;
	unsigned int sortKey;
public:
	CardSet(const QString &_shortName = QString(), const QString &_longName = QString());
	QString getShortName() const { return shortName; }
	QString getLongName() const { return longName; }
	int getSortKey() const { return sortKey; }
	void setSortKey(unsigned int _sortKey);
	void updateSortKey();
	void loadFromStream(QDataStream &stream);
	void saveToStream(QDataStream &stream);
};

class SetList : public QList<CardSet *> {
private:
	class CompareFunctor;
public:
	void sortByKey();
};

class CardInfo {
private:
	class SetCompareFunctor;
	CardDatabase *db;

	QString name;
	SetList sets;
	QString manacost;
	QString cardtype;
	QString powtough;
	QStringList text;
	QPixmap *pixmap;
	QMap<int, QPixmap *> scaledPixmapCache;
public:
	CardInfo(CardDatabase *_db,
		const QString &_name = QString(),
		const QString &_manacost = QString(),
		const QString &_cardtype = QString(),
		const QString &_powtough = QString(),
		const QStringList &_text = QStringList());
	~CardInfo();
	QString getName() const { return name; }
	SetList getSets() const { return sets; }
	QString getManacost() const { return manacost; }
	QString getCardType() const { return cardtype; }
	QString getPowTough() const { return powtough; }
	QStringList getText() const { return text; }
	QString getMainCardType() const;
	int getTableRow() const;
	void addToSet(CardSet *set);
	QPixmap *loadPixmap();
	QPixmap *getPixmap(QSize size);
	void loadFromStream(QDataStream &stream);
	void saveToStream(QDataStream &stream);
};

class CardDatabase {
private:
	QHash<QString, CardInfo *> cardHash;
	QHash<QString, CardSet *> setHash;
	CardInfo *noCard;
	static const unsigned int magicNumber = 0x12345678;
	static const unsigned int fileVersion = 1;
public:
	CardDatabase();
	~CardDatabase();
	void clear();
	CardInfo *getCard(const QString &cardName = QString());
	CardSet *getSet(const QString &setName);
	QList<CardInfo *> getCardList() const { return cardHash.values(); }
	SetList getSetList() const;
	void importOracleFile(const QString &fileName, CardSet *set);
	void importOracleDir();
	int loadFromFile(const QString &fileName);
	bool saveToFile(const QString &fileName);
};

#endif
