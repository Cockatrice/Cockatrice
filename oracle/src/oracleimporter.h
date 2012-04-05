#ifndef ORACLEIMPORTER_H
#define ORACLEIMPORTER_H

#include <carddatabase.h>
#include <QHttp>

class QBuffer;
class QXmlStreamReader;

class SetToDownload {
private:
	QString shortName, longName, url;
	bool import;
public:
	const QString &getShortName() const { return shortName; }
	const QString &getLongName() const { return longName; }
	const QString &getUrl() const { return url; }
	bool getImport() const { return import; }
	void setImport(bool _import) { import = _import; }
	SetToDownload(const QString &_shortName, const QString &_longName, const QString &_url, bool _import)
		: shortName(_shortName), longName(_longName), url(_url), import(_import) { }
};

class OracleImporter : public CardDatabase {
	Q_OBJECT
private:
	QList<SetToDownload> allSets, setsToDownload;
	QString pictureUrl, pictureUrlHq, pictureUrlSt, setUrl;
	QString dataDir;
	int setIndex;
	int reqId;
	QBuffer *buffer;
	QHttp *http;
	QString getPictureUrl(QString url, int cardId, QString name, const QString &setName) const;
	
	void downloadNextFile();
	bool readSetsFromXml(QXmlStreamReader &xml);
	CardInfo *addCard(const QString &setName, QString cardName, bool isToken, int cardId, const QString &cardCost, const QString &cardType, const QString &cardPT, int cardLoyalty, const QStringList &cardText);
private slots:
	void httpRequestFinished(int requestId, bool error);
	void readResponseHeader(const QHttpResponseHeader &responseHeader);
signals:
	void setIndexChanged(int cardsImported, int setIndex, const QString &nextSetName);
	void dataReadProgress(int bytesRead, int totalBytes);
public:
	OracleImporter(const QString &_dataDir, QObject *parent = 0);
	bool readSetsFromByteArray(const QByteArray &data);
	bool readSetsFromFile(const QString &fileName);
	int startDownload();
	int importTextSpoiler(CardSet *set, const QByteArray &data);
	QList<SetToDownload> &getSets() { return allSets; }
	const QString &getDataDir() const { return dataDir; }
};

#endif
