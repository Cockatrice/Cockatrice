#ifndef CARDDATABASE_H
#define CARDDATABASE_H

#include <QHash>
#include <QPixmap>
#include <QMap>
#include <QDataStream>
#include <QList>
#include <QXmlStreamReader>
#include <QHttp>
#include <QBuffer>

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
};

class SetList : public QList<CardSet *> {
private:
	class CompareFunctor;
public:
	void sortByKey();
};

class CardInfo : QObject {
	Q_OBJECT
private:
	CardDatabase *db;

	QString name;
	SetList sets;
	QString manacost;
	QString cardtype;
	QString powtough;
	QString text;
	QStringList colors;
	QString picURL;
	QHttp *http;
	QBuffer *downloadBuffer;
	int dlID;
	int tableRow;
	QPixmap *pixmap;
	QMap<int, QPixmap *> scaledPixmapCache;
	
	void startDownload();
public:
	CardInfo(CardDatabase *_db,
		const QString &_name = QString(),
		const QString &_manacost = QString(),
		const QString &_cardtype = QString(),
		const QString &_powtough = QString(),
		const QString &_text = QString(),
		const QStringList &_colors = QStringList(),
		int _tableRow = 0,
		const SetList &_sets = SetList(),
		const QString &_picURL = QString());
	~CardInfo();
	QString getName() const { return name; }
	SetList getSets() const { return sets; }
	QString getManaCost() const { return manacost; }
	QString getCardType() const { return cardtype; }
	QString getPowTough() const { return powtough; }
	QString getText() const { return text; }
	QStringList getColors() const { return colors; }
	QString getPicURL() const { return picURL; }
	QString getMainCardType() const;
	QString getCorrectedName() const;
	int getTableRow() const { return tableRow; }
	void setTableRow(int _tableRow) { tableRow = _tableRow; }
	void setPicURL(const QString &_picURL) { picURL = _picURL; }
	void addToSet(CardSet *set);
	QPixmap *loadPixmap();
	QPixmap *getPixmap(QSize size);
	void clearPixmapCache();
	void updatePixmapCache();
private slots:
	void picDownloadFinished(int id, bool error);
};

class CardDatabase : public QObject {
	Q_OBJECT
protected:
	QHash<QString, CardInfo *> cardHash;
	QHash<QString, CardSet *> setHash;
	CardInfo *noCard;
	QString picsPath, cardDatabasePath;
private:
	void loadCardsFromXml(QXmlStreamReader &xml);
	void loadSetsFromXml(QXmlStreamReader &xml);
	bool picDownload;
public:
	CardDatabase(QObject *parent = 0);
	~CardDatabase();
	void clear();
	CardInfo *getCard(const QString &cardName = QString());
	CardSet *getSet(const QString &setName);
	QList<CardInfo *> getCardList() const { return cardHash.values(); }
	SetList getSetList() const;
	bool getPicDownload() const { return picDownload; }
	void clearPixmapCache();
	int loadFromFile(const QString &fileName);
	bool saveToFile(const QString &fileName);
	const QString &getPicsPath() const { return picsPath; }
public slots:
	void updatePicsPath(const QString &path = QString());
	void updateDatabasePath(const QString &path = QString());
	void updatePicDownload(int _picDownload = -1);
};

#endif
