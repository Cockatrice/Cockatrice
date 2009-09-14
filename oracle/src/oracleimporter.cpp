#include "oracleimporter.h"
#include <QtGui>
#include <QtNetwork>
#include <QXmlStreamReader>

OracleImporter::OracleImporter(const QString &_dataDir)
	: dataDir(_dataDir), setIndex(-1)
{
	QFile setsFile(dataDir + "/sets.xml");
		setsFile.open(QIODevice::ReadOnly | QIODevice::Text);
	QXmlStreamReader xml(&setsFile);
	QString edition;
	QString editionLong;
	QString editionURL;
	while (!xml.atEnd()) {
		if (xml.readNext() == QXmlStreamReader::EndElement)
			break;
		if (xml.name() == "set") {
			QString shortName, longName;
			while (!xml.atEnd()) {
				if (xml.readNext() == QXmlStreamReader::EndElement)
					break;
				if (xml.name() == "name")
					edition = xml.readElementText();
				else if (xml.name() == "longname")
					editionLong = xml.readElementText();
				else if(xml.name() == "url")
					editionURL = xml.readElementText();
			}
			setsToDownload << SetToDownload(edition, editionLong, editionURL);
		} else if (xml.name() == "picture_url")
			pictureUrl = xml.readElementText();
	}
	
	buffer = new QBuffer(this);
	http = new QHttp(this);
	connect(http, SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
	connect(http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)), this, SLOT(readResponseHeader(const QHttpResponseHeader &)));
}

void OracleImporter::importOracleFile(CardSet *set)
{
	int cards = 0;
	buffer->seek(0);
	QTextStream in(buffer);
	while (!in.atEnd()) {
		QString cardname = in.readLine();
		if (cardname.isEmpty())
			continue;
		if (cardname.contains("XX")){
			cardname.remove("XX");
		}
		
		QString manacost = in.readLine();
		QString cardtype, powtough;
		QStringList text;
		if ((manacost.contains("Land")) || (manacost.contains("Sorcery")) || (manacost.contains("Instant")) || (manacost.contains("Artifact"))) {
			cardtype = manacost;
			manacost.clear();
		} else {
			cardtype = in.readLine();
			powtough = in.readLine();
			// Dirty hack.
			// Cards to test: Any creature, any basic land, Ancestral Vision, Fire // Ice.
			if (!powtough.contains("/") || powtough.size() > 5) {
				text << powtough;
				powtough = QString();
			}
		}
		QString line = in.readLine();
		QString firstTextLine = line;
		bool manaArtifact = false;
		while (!line.isEmpty()) {
			text << line;
			line = in.readLine();
		}
		// Table row override
		if (cardtype.endsWith("Artifact"))
			for (int i = 0; i < text.size(); ++i)
				if (text[i].contains("{T}") && text[i].contains("to your mana pool"))
					manaArtifact = true;
		CardInfo *card;
		if (cardHash.contains(cardname))
			card = cardHash.value(cardname);
		else {
			QStringList colors;
			QStringList allColors = QStringList() << "W" << "U" << "B" << "R" << "G";
			for (int i = 0; i < allColors.size(); i++)
				if (manacost.contains(allColors[i]))
					colors << allColors[i];

			QString wholeText = text.join(";");
			if (text.contains(cardname + " is white."))
				colors << "W";
			if (text.contains(cardname + " is blue."))
				colors << "U";
			if (text.contains(cardname + " is black."))
				colors << "B";
			if (text.contains(cardname + " is red."))
				colors << "R";
			if (text.contains(cardname + " is green."))
				colors << "G";
			
			card = new CardInfo(this, cardname, manacost, cardtype, powtough, text.join("\n"), colors);
			card->setPicURL(getURLFromName(normalizeName(cardname)));
			int tableRow = 1;
			QString mainCardType = card->getMainCardType();
			if ((mainCardType == "Land") || manaArtifact)
				tableRow = 0;
			else if ((mainCardType == "Sorcery") || (mainCardType == "Instant"))
				tableRow = 2;
			else if (mainCardType == "Creature")
				tableRow = 3;
			card->setTableRow(tableRow);

			cardHash.insert(cardname, card);
		}
		card->addToSet(set);
		cards++;
	}
	qDebug(QString("%1: %2 cards imported").arg(set->getLongName()).arg(cards).toLatin1());
}

QString OracleImporter::normalizeName(QString cardname)
{
	QString normalized = cardname;
	normalized.remove("'",Qt::CaseInsensitive);
	normalized.remove("//",Qt::CaseInsensitive);
	normalized.remove(",",Qt::CaseInsensitive);
	normalized.remove(":",Qt::CaseInsensitive);
	normalized.remove(".",Qt::CaseInsensitive);
	normalized.remove(QRegExp("\\(.*\\)"));
	normalized = normalized.trimmed();
	normalized = normalized.simplified();
	normalized = normalized.replace(" ", "_");
	normalized = normalized.replace("-", "_");
	return normalized;
}


QString OracleImporter::getURLFromName(QString normalizedName)
{
	return pictureUrl.arg(normalizedName);
}

void OracleImporter::downloadNextFile()
{
	if (setIndex == -1) {
		progressDialog = new QProgressDialog(tr("Downloading oracle files..."), QString(), 0, setsToDownload.size());
		setIndex = 0;
	}
	QString urlString = setsToDownload[setIndex].getUrl();
	if (urlString.startsWith("http://")) {
		QUrl url(urlString);
		http->setHost(url.host(), QHttp::ConnectionModeHttp, url.port() == -1 ? 0 : url.port());
		
		buffer->close();
		buffer->setData(QByteArray());
		buffer->open(QIODevice::ReadWrite | QIODevice::Text);
		reqId = http->get(QUrl::toPercentEncoding(url.path(), "!$&'()*+,;=:@/"), buffer);
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
	importOracleFile(set);
	progressDialog->setValue(++setIndex);
	
	if (setIndex == setsToDownload.size()) {
		setIndex = -1;
		saveToFile(dataDir + "/cards.xml");
		QMessageBox::information(0, tr("Import finished"), tr("Total: %1 cards imported").arg(cardHash.size()));
		qApp->quit();
	} else
		downloadNextFile();
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
			progressDialog->hide();
			http->abort();
			deleteLater();
	}
}
