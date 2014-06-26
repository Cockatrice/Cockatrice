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
#include <QWaitCondition>

class CardDatabase;
class CardInfo;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

typedef QMap<QString, QString> QStringMap;

// If we don't typedef this, CardInfo::CardInfo will refuse to compile on OS X.
typedef QMap<QString, int> MuidMap;

class CardSet : public QList<CardInfo *> {
private:
    QString shortName, longName;
    unsigned int sortKey;
public:
    CardSet(const QString &_shortName = QString(), const QString &_longName = QString());
    QString getCorrectedShortName() const;
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

class PictureToLoad {
private:
    CardInfo *card;
    bool stripped;
    SetList sortedSets;
    int setIndex;
    bool hq;
public:
    PictureToLoad(CardInfo *_card = 0, bool _stripped = false, bool _hq = true);
    CardInfo *getCard() const { return card; }
    bool getStripped() const { return stripped; }
    QString getSetName() const { return sortedSets[setIndex]->getCorrectedShortName(); }
    bool nextSet();
    bool getHq() const { return hq; }
    void setHq(bool _hq) { hq = _hq; }
};

class PictureLoader : public QObject {
    Q_OBJECT
private:
    QString _picsPath;
    QList<PictureToLoad> loadQueue;
    QMutex mutex;
    QNetworkAccessManager *networkManager;
    QList<PictureToLoad> cardsToDownload;
    PictureToLoad cardBeingDownloaded;
    bool picDownload, picDownloadHq, downloadRunning, loadQueueRunning;
    void startNextPicDownload();
    QString getPicUrl(CardInfo* card);
public:
    PictureLoader(const QString &__picsPath, bool _picDownload, bool _picDownloadHq, QObject *parent = 0);
    ~PictureLoader();
    void setPicsPath(const QString &path);
    void setPicDownload(bool _picDownload);
    void setPicDownloadHq(bool _picDownloadHq);
    void loadImage(CardInfo *card, bool stripped);
private slots:
    void picDownloadFinished(QNetworkReply *reply);
public slots:
    void processLoadQueue();
signals:
    void startLoadQueue();
    void imageLoaded(CardInfo *card, const QImage &image);
};

class CardInfo : public QObject {
    Q_OBJECT
private:
    CardDatabase *db;

    QString name;
    bool isToken;
    SetList sets;
    QString manacost;
    QString cardtype;
    QString powtough;
    QString text;
    QStringList colors;
    int loyalty;
    MuidMap muIds;
    bool cipt;
    int tableRow;
    QPixmap *pixmap;
    QMap<int, QPixmap *> scaledPixmapCache;
public:
    CardInfo(CardDatabase *_db,
        const QString &_name = QString(),
        bool _isToken = false,
        const QString &_manacost = QString(),
        const QString &_cardtype = QString(),
        const QString &_powtough = QString(),
        const QString &_text = QString(),
        const QStringList &_colors = QStringList(),
        int _loyalty = 0,
        bool _cipt = false,
        int _tableRow = 0,
        const SetList &_sets = SetList(),
        MuidMap muids = MuidMap());
    ~CardInfo();
    const QString &getName() const { return name; }
    bool getIsToken() const { return isToken; }
    const SetList &getSets() const { return sets; }
    const QString &getManaCost() const { return manacost; }
    const QString &getCardType() const { return cardtype; }
    const QString &getPowTough() const { return powtough; }
    const QString &getText() const { return text; }
    const int &getLoyalty() const { return loyalty; }
    bool getCipt() const { return cipt; }
    void setManaCost(const QString &_manaCost) { manacost = _manaCost; emit cardInfoChanged(this); }
    void setCardType(const QString &_cardType) { cardtype = _cardType; emit cardInfoChanged(this); }
    void setPowTough(const QString &_powTough) { powtough = _powTough; emit cardInfoChanged(this); }
    void setText(const QString &_text) { text = _text; emit cardInfoChanged(this); }
    void setColors(const QStringList &_colors) { colors = _colors; emit cardInfoChanged(this); }
    const QStringList &getColors() const { return colors; }
    int getMuId(const QString &set) const { return muIds.value(set); }
    QString getMainCardType() const;
    QString getCorrectedName() const;
    int getTableRow() const { return tableRow; }
    void setTableRow(int _tableRow) { tableRow = _tableRow; }
    void setLoyalty(int _loyalty) { loyalty = _loyalty; emit cardInfoChanged(this); }
    void setMuId(const QString &_set, const int &_muId) { muIds.insert(_set, _muId); }
    void addToSet(CardSet *set);
    QPixmap *loadPixmap();
    QPixmap *getPixmap(QSize size);
    void clearPixmapCache();
    void clearPixmapCacheMiss();
    void imageLoaded(const QImage &image);
    CardSet *getPreferredSet();
    int getPreferredMuId();
public slots:
    void updatePixmapCache();
signals:
    void pixmapUpdated();
    void cardInfoChanged(CardInfo *card);
};

enum LoadStatus { Ok, VersionTooOld, Invalid, NotLoaded, FileError, NoCards };

class CardDatabase : public QObject {
    Q_OBJECT
protected:
    QHash<QString, CardInfo *> cardHash;
    QHash<QString, CardSet *> setHash;
    CardInfo *noCard;

    QThread *pictureLoaderThread;
    PictureLoader *pictureLoader;
    LoadStatus loadStatus;
private:
    static const int versionNeeded;
    void loadCardsFromXml(QXmlStreamReader &xml);
    void loadSetsFromXml(QXmlStreamReader &xml);
public:
    CardDatabase(QObject *parent = 0);
    ~CardDatabase();
    void clear();
    void addCard(CardInfo *card);
    void removeCard(CardInfo *card);
    CardInfo *getCard(const QString &cardName = QString(), bool createIfNotFound = true);
    CardSet *getSet(const QString &setName);
    QList<CardInfo *> getCardList() const { return cardHash.values(); }
    SetList getSetList() const;
    LoadStatus loadFromFile(const QString &fileName, bool tokens = false);
    bool saveToFile(const QString &fileName, bool tokens = false);
    QStringList getAllColors() const;
    QStringList getAllMainCardTypes() const;
    LoadStatus getLoadStatus() const { return loadStatus; }
    bool getLoadSuccess() const { return loadStatus == Ok; }
    void cacheCardPixmaps(const QStringList &cardNames);
    void loadImage(CardInfo *card);
public slots:
    void clearPixmapCache();
    LoadStatus loadCardDatabase(const QString &path, bool tokens = false);
private slots:
    void imageLoaded(CardInfo *card, QImage image);
    void picDownloadChanged();
    void picDownloadHqChanged();
    void picsPathChanged();

    void loadCardDatabase();
    void loadTokenDatabase();
signals:
    void cardListChanged();
    void cardAdded(CardInfo *card);
    void cardRemoved(CardInfo *card);
};

#endif
