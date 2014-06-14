#include "oracleimporter.h"
#include <QtGui>
#include <QtNetwork>
#include <QXmlStreamReader>
#include <QDomDocument>
#include <QDebug>

#include "qt-json/json.h"

OracleImporter::OracleImporter(const QString &_dataDir, QObject *parent)
	: CardDatabase(parent), dataDir(_dataDir), setIndex(-1)
{
	buffer = new QBuffer(this);
	http = new QHttp(this);
	connect(http, SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
	connect(http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)), this, SLOT(readResponseHeader(const QHttpResponseHeader &)));
	connect(http, SIGNAL(dataReadProgress(int, int)), this, SIGNAL(dataReadProgress(int, int)));
}

bool OracleImporter::readSetsFromFile(const QString &fileName)
{
	QFile setsFile(fileName);
	if (!setsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::critical(0, tr("Error"), tr("Cannot open file '%1'.").arg(fileName));
		return false;
	}

	QXmlStreamReader xml(&setsFile);
	return readSetsFromXml(xml);
}

bool OracleImporter::readSetsFromByteArray(const QByteArray &data)
{
	QXmlStreamReader xml(data);
	return readSetsFromXml(xml);
}

bool OracleImporter::readSetsFromXml(QXmlStreamReader &xml)
{
	QList<SetToDownload> newSetList;
	
	QString edition;
	QString editionLong;
	QString editionURL;
	while (!xml.atEnd()) {
		if (xml.readNext() == QXmlStreamReader::EndElement)
			break;
		if (xml.name() == "set") {
			bool import = xml.attributes().value("import").toString().toInt();
			while (!xml.atEnd()) {
				if (xml.readNext() == QXmlStreamReader::EndElement)
					break;
				if (xml.name() == "name")
					edition = xml.readElementText();
				else if (xml.name() == "longname")
					editionLong = xml.readElementText();
				else if (xml.name() == "url")
					editionURL = xml.readElementText();
			}
			newSetList.append(SetToDownload(edition, editionLong, editionURL, import));
			edition = editionLong = editionURL = QString();
		} else if (xml.name() == "picture_url")
			pictureUrl = xml.readElementText();
		else if (xml.name() == "picture_url_hq")
			pictureUrlHq = xml.readElementText();
		else if (xml.name() == "picture_url_st")
			pictureUrlSt = xml.readElementText();
		else if (xml.name() == "set_url")
			setUrl = xml.readElementText();
	}
	if (newSetList.isEmpty())
		return false;
	allSets = newSetList;
	return true;
}

CardInfo *OracleImporter::addCard(const QString &setName,
								  QString cardName,
								  bool isToken,
								  int cardId,
								  const QString &cardCost,
								  const QString &cardType,
								  const QString &cardPT,
								  int cardLoyalty,
								  const QStringList &cardText)
{
	QString fullCardText = cardText.join("\n");
	bool splitCard = false;
	if (cardName.contains('(')) {
		cardName.remove(QRegExp(" \\(.*\\)"));
		splitCard = true;
	}
	// Workaround for card name weirdness
	if (cardName.contains("XX"))
		cardName.remove("XX");
	cardName = cardName.replace("Æ", "AE");
        cardName = cardName.replace("’", "'");

	CardInfo *card;
	if (cardHash.contains(cardName)) {
		card = cardHash.value(cardName);
		if (splitCard && !card->getText().contains(fullCardText))
			card->setText(card->getText() + "\n---\n" + fullCardText);
	} else {
		bool mArtifact = false;
		if (cardType.endsWith("Artifact"))
			for (int i = 0; i < cardText.size(); ++i)
				if (cardText[i].contains("{T}") && cardText[i].contains("to your mana pool"))
					mArtifact = true;
					
		QStringList colors;
		QStringList allColors = QStringList() << "W" << "U" << "B" << "R" << "G";
		for (int i = 0; i < allColors.size(); i++)
			if (cardCost.contains(allColors[i]))
				colors << allColors[i];
		
		if (cardText.contains(cardName + " is white."))
			colors << "W";
		if (cardText.contains(cardName + " is blue."))
			colors << "U";
		if (cardText.contains(cardName + " is black."))
			colors << "B";
		if (cardText.contains(cardName + " is red."))
			colors << "R";
		if (cardText.contains(cardName + " is green."))
			colors << "G";
		
		bool cipt = (cardText.contains(cardName + " enters the battlefield tapped."));
		
		card = new CardInfo(this, cardName, isToken, cardCost, cardType, cardPT, fullCardText, colors, cardLoyalty, cipt);
		int tableRow = 1;
		QString mainCardType = card->getMainCardType();
		if ((mainCardType == "Land") || mArtifact)
			tableRow = 0;
		else if ((mainCardType == "Sorcery") || (mainCardType == "Instant"))
			tableRow = 3;
		else if (mainCardType == "Creature")
			tableRow = 2;
		card->setTableRow(tableRow);
		
		cardHash.insert(cardName, card);
	}
	card->setPicURL(setName, getPictureUrl(pictureUrl, cardId, cardName, setName));
	card->setPicURLHq(setName, getPictureUrl(pictureUrlHq, cardId, cardName, setName));
	card->setPicURLSt(setName, getPictureUrl(pictureUrlSt, cardId, cardName, setName));
	return card;
}

int OracleImporter::importTextSpoiler(CardSet *set, const QByteArray &data)
{
	int cards = 0;
    bool ok;
    QVariantMap resultMap = QtJson::Json::parse(QString(data), ok).toMap();
    if (!ok) {
    	qDebug() << "error: QtJson::Json::parse()";
        return 0;
    }
    
    QListIterator<QVariant> it(resultMap.value("cards").toList());
    while (it.hasNext()) {
        QVariantMap map = it.next().toMap();
		QString cardName = map.value("name").toString();
		QString cardCost = map.value("manaCost").toString();
		QString cardType = map.value("type").toString();
		QString cardPT = map.value("power").toString() + QString('/') + map.value("toughness").toString();
		QString cardText = map.value("text").toString();
		int cardId = map.value("multiverseid").toInt();
		int cardLoyalty = map.value("loyalty").toInt();
		QStringList cardTextSplit = cardText.split("\n");

		CardInfo *card = addCard(set->getShortName(), cardName, false, cardId, cardCost, cardType, cardPT, cardLoyalty, cardTextSplit);

		if (!set->contains(card)) {
			card->addToSet(set);
			cards++;
		}
		cardName = cardCost = cardType = cardPT = cardText = QString();
    }
	
	return cards;
}

QString OracleImporter::getPictureUrl(QString url, int cardId, QString name, const QString &setName) const
{
	if ((name == "Island") || (name == "Swamp") || (name == "Mountain") || (name == "Plains") || (name == "Forest"))
		name.append("1");
	return url.replace("!cardid!", QString::number(cardId)).replace("!set!", setName).replace("!name!", name
		.replace("ö", "o")
//		.remove('\'')
		.remove(" // ")
//		.remove(',')
//		.remove(':')
//		.remove('.')
		.remove(QRegExp("\\(.*\\)"))
		.simplified()
//		.replace(' ', '_')
//		.replace('-', '_')
	);
}

int OracleImporter::startDownload()
{
	clear();
	
	setsToDownload.clear();
	for (int i = 0; i < allSets.size(); ++i)
		if (allSets[i].getImport())
			setsToDownload.append(allSets[i]);
	
	if (setsToDownload.isEmpty())
		return 0;
	setIndex = 0;
	emit setIndexChanged(0, 0, setsToDownload[0].getLongName());
	
	downloadNextFile();
	return setsToDownload.size();
}

void OracleImporter::downloadNextFile()
{
	QString urlString = setsToDownload[setIndex].getUrl();
	if (urlString.isEmpty())
		urlString = setUrl;
	urlString = urlString.replace("!name!", setsToDownload[setIndex].getShortName());

	if (urlString.startsWith("http://")) {
		QUrl url(urlString);
		http->setHost(url.host(), QHttp::ConnectionModeHttp, url.port() == -1 ? 0 : url.port());
		QString path = QUrl::toPercentEncoding(urlString.mid(url.host().size() + 7).replace(' ', '+'), "?!$&'()*+,;=:@/");
		
		buffer->close();
		buffer->setData(QByteArray());
		buffer->open(QIODevice::ReadWrite | QIODevice::Text);
		reqId = http->get(path, buffer);
	} else {
		QFile file(dataDir + "/" + urlString);
		file.open(QIODevice::ReadOnly | QIODevice::Text);
		
		buffer->close();
		buffer->setData(file.readAll());
		buffer->open(QIODevice::ReadWrite | QIODevice::Text);
		reqId = 0;
		httpRequestFinished(reqId, false);
	}
}

void OracleImporter::httpRequestFinished(int requestId, bool error)
{
	if (error) {
		QMessageBox::information(0, tr("HTTP"), tr("Error."));
		return;
	}
	if (requestId != reqId)
		return;

	CardSet *set = new CardSet(setsToDownload[setIndex].getShortName(), setsToDownload[setIndex].getLongName());
	if (!setHash.contains(set->getShortName()))
		setHash.insert(set->getShortName(), set);
	
	buffer->seek(0);
	buffer->close();
	int cards = importTextSpoiler(set, buffer->data());
        if (cards > 0)
            ++setIndex;
	
	if (setIndex == setsToDownload.size()) {
		emit setIndexChanged(cards, setIndex, QString());
		setIndex = -1;
	} else {
		downloadNextFile();
		emit setIndexChanged(cards, setIndex, setsToDownload[setIndex].getLongName());
	}
}

void OracleImporter::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
	switch (responseHeader.statusCode()) {
		case 200:
		case 301:
		case 302:
		case 303:
		case 307:
			break;
		default:
			QMessageBox::information(0, tr("HTTP"), tr("Download failed: %1.").arg(responseHeader.reasonPhrase()));
			http->abort();
			deleteLater();
	}
}
