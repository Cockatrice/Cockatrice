#ifndef CARDDATABASE_H
#define CARDDATABASE_H

#include <QHash>
#include <QPixmap>
#include <QMap>
#include <QDataStream>
#include <QList>
#include <QXmlStreamReader>
#include <QNetworkRequest>
#include <QThread>
#include <QMutex>

class CardDatabase;
class CardInfo;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

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

class PictureLoadingThread : public QThread {
	Q_OBJECT
private:
	QString _picsPath;
	QList<CardInfo *> loadQueue;
	QMutex mutex;
protected:
	void run();
public:
	PictureLoadingThread(QObject *parent);
	~PictureLoadingThread();
	void setPicsPath(const QString &path);
	void loadImage(CardInfo *card);
signals:
	void imageLoaded(CardInfo *card, const QImage &image);
};

class CardInfo : public QObject {
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
	QMap<QString, QString> picURLs;
	bool cipt;
	int tableRow;
	QPixmap *pixmap;
	QMap<int, QPixmap *> scaledPixmapCache;
public:
	CardInfo(CardDatabase *_db,
		const QString &_name = QString(),
		const QString &_manacost = QString(),
		const QString &_cardtype = QString(),
		const QString &_powtough = QString(),
		const QString &_text = QString(),
		const QStringList &_colors = QStringList(),
		bool cipt = false,
		int _tableRow = 0,
		const SetList &_sets = SetList(),
		const QMap<QString, QString> &_picURLs = QMap<QString, QString>());
	~CardInfo();
	const QString &getName() const { return name; }
	const SetList &getSets() const { return sets; }
	const QString &getManaCost() const { return manacost; }
	const QString &getCardType() const { return cardtype; }
	const QString &getPowTough() const { return powtough; }
	const QString &getText() const { return text; }
	bool getCipt() const { return cipt; }
	void setText(const QString &_text) { text = _text; }
	const QStringList &getColors() const { return colors; }
	QString getPicURL(const QString &set) const { return picURLs.value(set); }
	QString getPicURL() const;
	const QMap<QString, QString> &getPicURLs() const { return picURLs; }
	QString getMainCardType() const;
	QString getCorrectedName() const;
	int getTableRow() const { return tableRow; }
	void setTableRow(int _tableRow) { tableRow = _tableRow; }
	void setPicURL(const QString &_set, const QString &_picURL) { picURLs.insert(_set, _picURL); }
	void addToSet(CardSet *set);
	QPixmap *loadPixmap();
	QPixmap *getPixmap(QSize size);
	void clearPixmapCache();
	void clearPixmapCacheMiss();
	void imageLoaded(const QImage &image);
public slots:
	void updatePixmapCache();
signals:
	void pixmapUpdated();
};

class CardDatabase : public QObject {
	Q_OBJECT
protected:
	QHash<QString, CardInfo *> cardHash;
	QHash<QString, CardSet *> setHash;
	QNetworkAccessManager *networkManager;
	QList<CardInfo *> cardsToDownload;
	CardInfo *cardBeingDownloaded;
	bool downloadRunning;
	bool loadSuccess;
	CardInfo *noCard;
	PictureLoadingThread *loadingThread;
private:
	void loadCardsFromXml(QXmlStreamReader &xml);
	void loadSetsFromXml(QXmlStreamReader &xml);
	void startNextPicDownload();
public:
	CardDatabase(QObject *parent = 0);
	~CardDatabase();
	void clear();
	CardInfo *getCard(const QString &cardName = QString());
	CardSet *getSet(const QString &setName);
	QList<CardInfo *> getCardList() const { return cardHash.values(); }
	SetList getSetList() const;
	bool loadFromFile(const QString &fileName);
	bool saveToFile(const QString &fileName);
	void startPicDownload(CardInfo *card);
	QStringList getAllColors() const;
	QStringList getAllMainCardTypes() const;
	bool getLoadSuccess() const { return loadSuccess; }
	void cacheCardPixmaps(const QStringList &cardNames);
	void loadImage(CardInfo *card);
public slots:
	void clearPixmapCache();
	bool loadCardDatabase(const QString &path);
	bool loadCardDatabase();
private slots:
	void picDownloadFinished(QNetworkReply *reply);
	void picDownloadChanged();
	void imageLoaded(CardInfo *card, QImage image);
	void picsPathChanged();
};

#endif
