#include "oracleimporter.h"
#include <QtGui>
#include <QtNetwork>

OracleImporter::OracleImporter()
	: setIndex(-1)
{
	setsToDownload << SetToDownload("10E", "Tenth Edition", "http://www.crystalkeep.com/magic/rules/oracle/oracle-10.txt");
	setsToDownload << SetToDownload("ARB", "Alara Reborn", "http://www.crystalkeep.com/magic/rules/oracle/oracle-ar.txt");
	setsToDownload << SetToDownload("CFX", "Conflux", "http://www.crystalkeep.com/magic/rules/oracle/oracle-cf.txt");
	setsToDownload << SetToDownload("ALA", "Shards of Alara", "http://www.crystalkeep.com/magic/rules/oracle/oracle-sa.txt");
	setsToDownload << SetToDownload("EVE", "Eventide", "http://www.crystalkeep.com/magic/rules/oracle/oracle-et.txt");
	setsToDownload << SetToDownload("SHM", "Shadowmoor", "http://www.crystalkeep.com/magic/rules/oracle/oracle-sm.txt");
	setsToDownload << SetToDownload("MOR", "Morningtide", "http://www.crystalkeep.com/magic/rules/oracle/oracle-mt.txt");
	setsToDownload << SetToDownload("LRW", "Lorwyn", "http://www.crystalkeep.com/magic/rules/oracle/oracle-lo.txt");
	setsToDownload << SetToDownload("9E", "Ninth Edition", "http://www.crystalkeep.com/magic/rules/oracle/oracle-9th.txt");
	setsToDownload << SetToDownload("8E", "Eighth Edition", "http://www.crystalkeep.com/magic/rules/oracle/oracle-8th.txt");
	setsToDownload << SetToDownload("FUT", "Future Sight", "http://www.crystalkeep.com/magic/rules/oracle/oracle-fs.txt");
	setsToDownload << SetToDownload("PLC", "Planar Chaos", "http://www.crystalkeep.com/magic/rules/oracle/oracle-pc.txt");
	setsToDownload << SetToDownload("TSP", "Time Spiral", "http://www.crystalkeep.com/magic/rules/oracle/oracle-ts.txt");
	setsToDownload << SetToDownload("CS", "Coldsnap", "http://www.crystalkeep.com/magic/rules/oracle/oracle-cs.txt");
	setsToDownload << SetToDownload("DIS", "Dissension", "http://www.crystalkeep.com/magic/rules/oracle/oracle-di.txt");
	setsToDownload << SetToDownload("GP", "Guildpact", "http://www.crystalkeep.com/magic/rules/oracle/oracle-gp.txt");
	setsToDownload << SetToDownload("RAV", "Ravnica", "http://www.crystalkeep.com/magic/rules/oracle/oracle-ra.txt");
	setsToDownload << SetToDownload("SOK", "Saviors of Kamigawa", "http://www.crystalkeep.com/magic/rules/oracle/oracle-sk.txt");
	setsToDownload << SetToDownload("BOK", "Betrayers of Kamigawa", "http://www.crystalkeep.com/magic/rules/oracle/oracle-bk.txt");
	setsToDownload << SetToDownload("CHK", "Champions of Kamigawa", "http://www.crystalkeep.com/magic/rules/oracle/oracle-ck.txt");
	setsToDownload << SetToDownload("FD", "Fifth Dawn", "http://www.crystalkeep.com/magic/rules/oracle/oracle-fd.txt");
	setsToDownload << SetToDownload("DS", "Darksteel", "http://www.crystalkeep.com/magic/rules/oracle/oracle-ds.txt");
	setsToDownload << SetToDownload("MR", "Mirrodin", "http://www.crystalkeep.com/magic/rules/oracle/oracle-mr.txt");
	setsToDownload << SetToDownload("SC", "Scourge", "http://www.crystalkeep.com/magic/rules/oracle/oracle-sc.txt");
	setsToDownload << SetToDownload("LE", "Legions", "http://www.crystalkeep.com/magic/rules/oracle/oracle-le.txt");
	setsToDownload << SetToDownload("ON", "Onslaught", "http://www.crystalkeep.com/magic/rules/oracle/oracle-on.txt");
	setsToDownload << SetToDownload("7E", "Seventh Edition", "http://www.crystalkeep.com/magic/rules/oracle/oracle-7th.txt");
	setsToDownload << SetToDownload("6E", "Sixth Edition", "http://www.crystalkeep.com/magic/rules/oracle/oracle-6th.txt");
	setsToDownload << SetToDownload("5E", "Fifth Edition", "http://www.crystalkeep.com/magic/rules/oracle/oracle-5th.txt");
	setsToDownload << SetToDownload("4E", "Fourth Edition", "http://www.crystalkeep.com/magic/rules/oracle/oracle-4th.txt");
	setsToDownload << SetToDownload("R", "Revised Edition", "http://www.crystalkeep.com/magic/rules/oracle/oracle-rv.txt");
	setsToDownload << SetToDownload("U", "Unlimited Edition", "http://www.crystalkeep.com/magic/rules/oracle/oracle-lu.txt");
	setsToDownload << SetToDownload("B", "Limited Edition", "http://www.crystalkeep.com/magic/rules/oracle/oracle-lu.txt");
	setsToDownload << SetToDownload("JU", "Judgment", "http://www.crystalkeep.com/magic/rules/oracle/oracle-ju.txt");
	setsToDownload << SetToDownload("TO", "Torment", "http://www.crystalkeep.com/magic/rules/oracle/oracle-to.txt");
	setsToDownload << SetToDownload("OD", "Odyssey", "http://www.crystalkeep.com/magic/rules/oracle/oracle-od.txt");
	setsToDownload << SetToDownload("AP", "Apocalypse", "http://www.crystalkeep.com/magic/rules/oracle/oracle-ap.txt");
	setsToDownload << SetToDownload("PS", "Planeshift", "http://www.crystalkeep.com/magic/rules/oracle/oracle-ps.txt");
	setsToDownload << SetToDownload("IN", "Invasion", "http://www.crystalkeep.com/magic/rules/oracle/oracle-in.txt");
	setsToDownload << SetToDownload("PY", "Prophecy", "http://www.crystalkeep.com/magic/rules/oracle/oracle-py.txt");
	setsToDownload << SetToDownload("NE", "Nemesis", "http://www.crystalkeep.com/magic/rules/oracle/oracle-ne.txt");
	setsToDownload << SetToDownload("MM", "Mercadian Masques", "http://www.crystalkeep.com/magic/rules/oracle/oracle-mm.txt");
	setsToDownload << SetToDownload("UD", "Urza's Destiny", "http://www.crystalkeep.com/magic/rules/oracle/oracle-ud.txt");
	setsToDownload << SetToDownload("UL", "Urza's Legacy", "http://www.crystalkeep.com/magic/rules/oracle/oracle-ul.txt");
	setsToDownload << SetToDownload("US", "Urza's Saga", "http://www.crystalkeep.com/magic/rules/oracle/oracle-us.txt");
	setsToDownload << SetToDownload("EX", "Exodus", "http://www.crystalkeep.com/magic/rules/oracle/oracle-ex.txt");
	setsToDownload << SetToDownload("SH", "Stronghold", "http://www.crystalkeep.com/magic/rules/oracle/oracle-sh.txt");
	setsToDownload << SetToDownload("TE", "Tempest", "http://www.crystalkeep.com/magic/rules/oracle/oracle-te.txt");
	setsToDownload << SetToDownload("WL", "Weatherlight", "http://www.crystalkeep.com/magic/rules/oracle/oracle-wl.txt");
	setsToDownload << SetToDownload("VI", "Visions", "http://www.crystalkeep.com/magic/rules/oracle/oracle-vi.txt");
	setsToDownload << SetToDownload("MI", "Mirage", "http://www.crystalkeep.com/magic/rules/oracle/oracle-mi.txt");
	setsToDownload << SetToDownload("AL", "Alliances", "http://www.crystalkeep.com/magic/rules/oracle/oracle-al.txt");
	setsToDownload << SetToDownload("HL", "Homelands", "http://www.crystalkeep.com/magic/rules/oracle/oracle-hl.txt");
	setsToDownload << SetToDownload("IA", "Ice Age", "http://www.crystalkeep.com/magic/rules/oracle/oracle-ia.txt");
	setsToDownload << SetToDownload("CH", "Chronicles", "http://www.crystalkeep.com/magic/rules/oracle/oracle-ch.txt");
	setsToDownload << SetToDownload("FE", "Fallen Empires", "http://www.crystalkeep.com/magic/rules/oracle/oracle-fe.txt");
	setsToDownload << SetToDownload("DK", "The Dark", "http://www.crystalkeep.com/magic/rules/oracle/oracle-dk.txt");
	setsToDownload << SetToDownload("LG", "Legends", "http://www.crystalkeep.com/magic/rules/oracle/oracle-lg.txt");
	setsToDownload << SetToDownload("AQ", "Antiquities", "http://www.crystalkeep.com/magic/rules/oracle/oracle-aq.txt");
	setsToDownload << SetToDownload("AN", "Arabian Nights", "http://www.crystalkeep.com/magic/rules/oracle/oracle-an.txt");
	setsToDownload << SetToDownload("PT", "Portal", "http://www.crystalkeep.com/magic/rules/oracle/oracle-pt.txt");
	setsToDownload << SetToDownload("P2", "Portal: Second Age", "http://www.crystalkeep.com/magic/rules/oracle/oracle-pt2.txt");
	setsToDownload << SetToDownload("P3", "Portal: Three Kingdoms", "http://www.crystalkeep.com/magic/rules/oracle/oracle-pt3.txt");
	setsToDownload << SetToDownload("ST", "Starter", "http://www.crystalkeep.com/magic/rules/oracle/oracle-st.txt");
	setsToDownload << SetToDownload("ST2K", "Starter 2000", "http://www.crystalkeep.com/magic/rules/oracle/oracle-st2.txt");
	setsToDownload << SetToDownload("PR", "Promo cards", "pr.txt");
	setsToDownload << SetToDownload("UG", "Unglued", "http://www.crystalkeep.com/magic/rules/oracle/oracle-ug.txt");
	setsToDownload << SetToDownload("UNH", "Unhinged", "http://www.crystalkeep.com/magic/rules/oracle/oracle-uh.txt");

	colorOverride.insert("Ancestral Vision", QStringList("U"));
	colorOverride.insert("Crimson Kobolds", QStringList("R"));
	colorOverride.insert("Crookshank Kobolds", QStringList("R"));
	colorOverride.insert("Dryad Arbor", QStringList("G"));
	colorOverride.insert("Evermind", QStringList("U"));
	colorOverride.insert("Hypergenesis", QStringList("G"));
	colorOverride.insert("Intervention Pact", QStringList("W"));
	colorOverride.insert("Kobolds of Kher Keep", QStringList("R"));
	colorOverride.insert("Living End", QStringList("B"));
	colorOverride.insert("Pact of Negation", QStringList("U"));
	colorOverride.insert("Pact of the Titan", QStringList("R"));
	colorOverride.insert("Restore Balance", QStringList("W"));
	colorOverride.insert("Slaughter Pact", QStringList("B"));
	colorOverride.insert("Summoner's Pact", QStringList("G"));
	colorOverride.insert("Wheel of Fate", QStringList("R"));

	tableRowOverride.insert("Azorius Signet", 0);
	tableRowOverride.insert("Basalt Monolith", 0);
	tableRowOverride.insert("Black Lotus", 0);
	tableRowOverride.insert("Black Mana Battery", 0);
	tableRowOverride.insert("Bloodstone Cameo", 0);
	tableRowOverride.insert("Blue Mana Battery", 0);
	tableRowOverride.insert("Boros Signet", 0);
	tableRowOverride.insert("Celestial Prism", 0);
	tableRowOverride.insert("Charcoal Diamond", 0);
	tableRowOverride.insert("Chrome Mox", 0);
	tableRowOverride.insert("Chronatog Totem", 0);
	tableRowOverride.insert("Coalition Relic", 0);
	tableRowOverride.insert("Coldsteel Heart", 0);
	tableRowOverride.insert("Darksteel Ingot", 0);
	tableRowOverride.insert("Dimir Signet", 0);
	tableRowOverride.insert("Drake-Skull Cameo", 0);
	tableRowOverride.insert("Eye of Ramos", 0);
	tableRowOverride.insert("Fellwar Stone", 0);
	tableRowOverride.insert("Fieldmist Borderpost", 0);
	tableRowOverride.insert("Fire Diamond", 0);
	tableRowOverride.insert("Firewild Borderpost", 0);
	tableRowOverride.insert("Foriysian Totem", 0);
	tableRowOverride.insert("Gemstone Array", 0);
	tableRowOverride.insert("Gilded Lotus", 0);
	tableRowOverride.insert("Golgari Signet", 0);
	tableRowOverride.insert("Green Mana Battery", 0);
	tableRowOverride.insert("Grim Monolith", 0);
	tableRowOverride.insert("Gruul Signet", 0);
	tableRowOverride.insert("Guardian Idol", 0);
	tableRowOverride.insert("Heart of Ramos", 0);
	tableRowOverride.insert("Honor-Worn Shaku", 0);
	tableRowOverride.insert("Horn of Ramos", 0);
	tableRowOverride.insert("Izzet Signet", 0);
	tableRowOverride.insert("Jeweled Amulet", 0);
	tableRowOverride.insert("Kyren Toy", 0);
	tableRowOverride.insert("Lion's Eye Diamond", 0);
	tableRowOverride.insert("Lotus Bloom", 0);
	tableRowOverride.insert("Lotus Blossom", 0);
	tableRowOverride.insert("Lotus Petal", 0);
	tableRowOverride.insert("Mana Crypt", 0);
	tableRowOverride.insert("Mana Cylix", 0);
	tableRowOverride.insert("Mana Prism", 0);
	tableRowOverride.insert("Mana Vault", 0);
	tableRowOverride.insert("Marble Diamond", 0);
	tableRowOverride.insert("Mind Stone", 0);
	tableRowOverride.insert("Mistvein Borderpost", 0);
	tableRowOverride.insert("Moss Diamond", 0);
	tableRowOverride.insert("Mox Diamond", 0);
	tableRowOverride.insert("Mox Emerald", 0);
	tableRowOverride.insert("Mox Jet", 0);
	tableRowOverride.insert("Mox Pearl", 0);
	tableRowOverride.insert("Mox Ruby", 0);
	tableRowOverride.insert("Mox Sapphire", 0);
	tableRowOverride.insert("Obelisk of Bant", 0);
	tableRowOverride.insert("Obelisk of Esper", 0);
	tableRowOverride.insert("Obelisk of Grixis", 0);
	tableRowOverride.insert("Obelisk of Jund", 0);
	tableRowOverride.insert("Obelisk of Naya", 0);
	tableRowOverride.insert("Orzhov Signet", 0);
	tableRowOverride.insert("Paradise Plume", 0);
	tableRowOverride.insert("Pentad Prism", 0);
	tableRowOverride.insert("Phyrexian Lens", 0);
	tableRowOverride.insert("Phyrexian Totem", 0);
	tableRowOverride.insert("Prismatic Lens", 0);
	tableRowOverride.insert("Rakdos Signet", 0);
	tableRowOverride.insert("Red Mana Battery", 0);
	tableRowOverride.insert("Seashell Cameo", 0);
	tableRowOverride.insert("Selesnya Signet", 0);
	tableRowOverride.insert("Serum Powder", 0);
	tableRowOverride.insert("Simic Signet", 0);
	tableRowOverride.insert("Sisay's Ring", 0);
	tableRowOverride.insert("Skull of Ramos", 0);
	tableRowOverride.insert("Sky Diamond", 0);
	tableRowOverride.insert("Sol Grail", 0);
	tableRowOverride.insert("Sol Ring", 0);
	tableRowOverride.insert("Spectral Searchlight", 0);
	tableRowOverride.insert("Springleaf Drum", 0);
	tableRowOverride.insert("Standing Stones", 0);
	tableRowOverride.insert("Star Compass", 0);
	tableRowOverride.insert("Talisman of Dominance", 0);
	tableRowOverride.insert("Talisman of Impulse", 0);
	tableRowOverride.insert("Talisman of Indulgence", 0);
	tableRowOverride.insert("Talisman of Progress", 0);
	tableRowOverride.insert("Talisman of Unity", 0);
	tableRowOverride.insert("Terrarion", 0);
	tableRowOverride.insert("Thran Dynamo", 0);
	tableRowOverride.insert("Thunder Totem", 0);
	tableRowOverride.insert("Tigereye Cameo", 0);
	tableRowOverride.insert("Tooth of Ramos", 0);
	tableRowOverride.insert("Troll-Horn Cameo", 0);
	tableRowOverride.insert("Ur-Golem's Eye", 0);
	tableRowOverride.insert("Veinfire Borderpost", 0);
	tableRowOverride.insert("Weatherseed Totem", 0);
	tableRowOverride.insert("White Mana Battery", 0);
	tableRowOverride.insert("Wildfield Borderpost", 0);
	tableRowOverride.insert("Worn Powerstone", 0);
	
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
		QString manacost = in.readLine();
		QString cardtype, powtough;
		QStringList text;
		if ((manacost.contains("Land")) || (manacost.contains("Sorcery")) || (manacost.contains("Instant"))) {
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
		while (!line.isEmpty()) {
			text << line;
			line = in.readLine();
		}
		CardInfo *card;
		if (cardHash.contains(cardname))
			card = cardHash.value(cardname);
		else {
			QStringList colors;
			QStringList allColors = QStringList() << "W" << "U" << "B" << "R" << "G";
			for (int i = 0; i < allColors.size(); i++)
				if (manacost.contains(allColors[i]))
					colors << allColors[i];
			
			if (colorOverride.contains(cardname))
				colors = colorOverride.value(cardname);
				
			card = new CardInfo(this, cardname, manacost, cardtype, powtough, text.join("\n"), colors);

			int tableRow = 1;
			QString mainCardType = card->getMainCardType();
			if (tableRowOverride.contains(cardname))
				tableRow = tableRowOverride.value(cardname);
			else if (mainCardType == "Land")
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
		QFile file(urlString);
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
		qDebug(QString("Total: %1 cards imported").arg(cardHash.size()).toLatin1());
		setIndex = -1;
		saveToFile("cards.xml");
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
