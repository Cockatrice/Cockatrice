#ifndef ORACLEIMPORTER_H
#define ORACLEIMPORTER_H

#include <QMap>

#include <carddatabase.h>

class SetToDownload {
private:
	QString shortName, longName;
	bool import;
	QVariant cards;
public:
	const QString &getShortName() const { return shortName; }
	const QString &getLongName() const { return longName; }
	const QVariant &getCards() const { return cards; }
	bool getImport() const { return import; }
	void setImport(bool _import) { import = _import; }
	SetToDownload(const QString &_shortName, const QString &_longName, const QVariant &_cards, bool _import)
		: shortName(_shortName), longName(_longName), cards(_cards), import(_import) { }
};

class OracleImporter : public CardDatabase {
	Q_OBJECT
private:
	QList<SetToDownload> allSets;
	QVariantMap setsMap;
	QString dataDir;
	
	void downloadNextFile();
	CardInfo *addCard(const QString &setName, QString cardName, bool isToken, int cardId, const QString &cardCost, const QString &cardType, const QString &cardPT, int cardLoyalty, const QStringList &cardText);
signals:
	void setIndexChanged(int cardsImported, int setIndex, const QString &nextSetName);
	void dataReadProgress(int bytesRead, int totalBytes);
public:
	OracleImporter(const QString &_dataDir, QObject *parent = 0);
	bool readSetsFromByteArray(const QByteArray &data);
	bool readSetsFromFile(const QString &fileName);
	int startImport();
	int importTextSpoiler(CardSet *set, const QVariant &data);
	QList<SetToDownload> &getSets() { return allSets; }
	const QString &getDataDir() const { return dataDir; }
};

#endif
