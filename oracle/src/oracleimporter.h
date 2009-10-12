#ifndef ORACLEIMPORTER_H
#define ORACLEIMPORTER_H

#include <carddatabase.h>
#include <QHttp>

class QBuffer;

class SetToDownload {
private:
	QString shortName, longName, url;
public:
	const QString &getShortName() const { return shortName; }
	const QString &getLongName() const { return longName; }
	const QString &getUrl() const { return url; }
	SetToDownload(const QString &_shortName, const QString &_longName, const QString &_url)
		: shortName(_shortName), longName(_longName), url(_url) { }
};

class OracleImporter : public CardDatabase {
	Q_OBJECT
private:
	QList<SetToDownload> setsToDownload;
	QString pictureUrl, setUrl;
	QString dataDir;
	int setIndex;
	int reqId;
	QBuffer *buffer;
	QHttp *http;
	QString normalizeName(QString);
	QString getURLFromName(QString);
	
	CardInfo *addCard(QString cardName, const QString &cardCost, const QString &cardType, const QString &cardPT, const QStringList &cardText);
private slots:
	void httpRequestFinished(int requestId, bool error);
	void readResponseHeader(const QHttpResponseHeader &responseHeader);
signals:
	void setIndexChanged(int cardsImported, int setIndex, const QString &nextSetName);
	void dataReadProgress(int bytesRead, int totalBytes);
public:
	OracleImporter(const QString &_dataDir, QObject *parent = 0);
	int importTextSpoiler(CardSet *set, const QByteArray &data);
	void downloadNextFile();
	int getSetsCount() const { return setsToDownload.size(); }
};

#endif
