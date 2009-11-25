#include "protocol_datastructures.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class ColorConverter {
public:
	static int colorToInt(const QColor &color)
	{
		return color.red() * 65536 + color.green() * 256 + color.blue();
	}
	static QColor colorFromInt(int colorValue)
	{
		return QColor(colorValue / 65536, (colorValue % 65536) / 256, colorValue % 256);
	}
};

ServerInfo_Player::~ServerInfo_Player()
{
	for (int i = 0; i < zoneList.size(); ++i)
		delete zoneList[i];
	for (int i = 0; i < arrowList.size(); ++i)
		delete arrowList[i];
	for (int i = 0; i < counterList.size(); ++i)
		delete counterList[i];
}

ServerInfo_Zone::~ServerInfo_Zone()
{
	for (int i = 0; i < cardList.size(); ++i)
		delete cardList[i];
}

bool ServerInfo_Arrow::readElement(QXmlStreamReader *xml)
{
	if (xml->isStartElement() && (xml->name() == "arrow")) {
		id = xml->attributes().value("id").toString().toInt();
		startPlayerId = xml->attributes().value("start_player_id").toString().toInt();
		startZone = xml->attributes().value("start_zone").toString();
		startCardId = xml->attributes().value("start_card_id").toString().toInt();
		targetPlayerId = xml->attributes().value("target_player_id").toString().toInt();
		targetZone = xml->attributes().value("target_zone").toString();
		targetCardId = xml->attributes().value("target_card_id").toString().toInt();
		color = ColorConverter::colorFromInt(xml->attributes().value("color").toString().toInt());
	} else if (xml->isEndElement() && (xml->name() == "arrow"))
		return true;
	return false;
}

void ServerInfo_Arrow::writeElement(QXmlStreamWriter *xml)
{
	xml->writeStartElement("arrow");
	xml->writeAttribute("id", QString::number(id));
	xml->writeAttribute("start_player_id", QString::number(startPlayerId));
	xml->writeAttribute("start_zone", startZone);
	xml->writeAttribute("start_card_id", QString::number(startCardId));
	xml->writeAttribute("target_player_id", QString::number(targetPlayerId));
	xml->writeAttribute("target_zone", targetZone);
	xml->writeAttribute("target_card_id", QString::number(targetCardId));
	xml->writeAttribute("color", QString::number(ColorConverter::colorToInt(color)));
	xml->writeEndElement();
}

bool ServerInfo_Counter::readElement(QXmlStreamReader *xml)
{
	if (xml->isStartElement() && (xml->name() == "counter")) {
		id = xml->attributes().value("id").toString().toInt();
		name = xml->attributes().value("name").toString();
		color = ColorConverter::colorFromInt(xml->attributes().value("color").toString().toInt());
		radius = xml->attributes().value("radius").toString().toInt();
		count = xml->attributes().value("count").toString().toInt();
	} else if (xml->isEndElement() && (xml->name() == "counter"))
		return true;
	return false;
}

void ServerInfo_Counter::writeElement(QXmlStreamWriter *xml)
{
	xml->writeStartElement("counter");
	xml->writeAttribute("id", QString::number(id));
	xml->writeAttribute("name", name);
	xml->writeAttribute("color", QString::number(ColorConverter::colorToInt(color)));
	xml->writeAttribute("radius", QString::number(radius));
	xml->writeAttribute("count", QString::number(count));
	xml->writeEndElement();
}

bool ServerInfo_Card::readElement(QXmlStreamReader *xml)
{
	if (xml->isStartElement() && (xml->name() == "card")) {
		id = xml->attributes().value("id").toString().toInt();
		name = xml->attributes().value("name").toString();
		x = xml->attributes().value("x").toString().toInt();
		y = xml->attributes().value("y").toString().toInt();
		counters = xml->attributes().value("counters").toString().toInt();
		tapped = xml->attributes().value("tapped").toString().toInt();
		attacking = xml->attributes().value("attacking").toString().toInt();
		annotation = xml->attributes().value("annotation").toString();
	} else if (xml->isEndElement() && (xml->name() == "card"))
		return true;
	return false;
}

void ServerInfo_Card::writeElement(QXmlStreamWriter *xml)
{
	xml->writeStartElement("card");
	xml->writeAttribute("id", QString::number(id));
	xml->writeAttribute("name", name);
	xml->writeAttribute("x", QString::number(x));
	xml->writeAttribute("y", QString::number(y));
	xml->writeAttribute("counters", QString::number(counters));
	xml->writeAttribute("tapped", tapped ? "1" : "0");
	xml->writeAttribute("attacking", attacking ? "1" : "0");
	xml->writeAttribute("annotation", annotation);
	xml->writeEndElement();
}

bool ServerInfo_Zone::readElement(QXmlStreamReader *xml)
{
	if (currentItem) {
		if (currentItem->readElement(xml))
			currentItem = 0;
		return false;
	}
	if (xml->isStartElement() && (xml->name() == "zone")) {
		name = xml->attributes().value("name").toString();
		type = (ZoneType) xml->attributes().value("type").toString().toInt();
		hasCoords = xml->attributes().value("has_coords").toString().toInt();
		cardCount = xml->attributes().value("card_count").toString().toInt();
	} else if (xml->isStartElement() && (xml->name() == "card")) {
		ServerInfo_Card *card = new ServerInfo_Card;
		cardList.append(card);
		currentItem = card;
	} else if (xml->isEndElement() && (xml->name() == "zone"))
		return true;

	if (currentItem)
		if (currentItem->readElement(xml))
			currentItem = 0;
	return false;

}

void ServerInfo_Zone::writeElement(QXmlStreamWriter *xml)
{
	xml->writeStartElement("zone");
	xml->writeAttribute("name", name);
	QString typeStr;
	switch (type) {
		case PrivateZone: typeStr = "private"; break;
		case HiddenZone: typeStr = "hidden"; break;
		case PublicZone: typeStr = "public"; break;
	}
	xml->writeAttribute("type", typeStr);
	xml->writeAttribute("has_coords", hasCoords ? "1" : "0");
	xml->writeAttribute("card_count", QString::number(cardCount));
	for (int i = 0; i < cardList.size(); ++i)
		cardList[i]->writeElement(xml);
	xml->writeEndElement();
}

bool ServerInfo_Player::readElement(QXmlStreamReader *xml)
{
	if (currentItem) {
		if (currentItem->readElement(xml))
			currentItem = 0;
		return false;
	}
	if (xml->isStartElement() && (xml->name() == "player")) {
		playerId = xml->attributes().value("player_id").toString().toInt();
		name = xml->attributes().value("name").toString();
	} else if (xml->isStartElement() && (xml->name() == "zone")) {
		ServerInfo_Zone *zone = new ServerInfo_Zone;
		zoneList.append(zone);
		currentItem = zone;
	} else if (xml->isStartElement() && (xml->name() == "counter")) {
		ServerInfo_Counter *counter = new ServerInfo_Counter;
		counterList.append(counter);
		currentItem = counter;
	} else if (xml->isStartElement() && (xml->name() == "arrow")) {
		ServerInfo_Arrow *arrow = new ServerInfo_Arrow;
		arrowList.append(arrow);
		currentItem = arrow;
	} else if (xml->isEndElement() && (xml->name() == "player"))
		return true;

	if (currentItem)
		if (currentItem->readElement(xml))
			currentItem = 0;
	return false;
}

void ServerInfo_Player::writeElement(QXmlStreamWriter *xml)
{
	xml->writeStartElement("player");
	xml->writeAttribute("player_id", QString::number(playerId));
	xml->writeAttribute("name", name);
	for (int i = 0; i < zoneList.size(); ++i)
		zoneList[i]->writeElement(xml);
	for (int i = 0; i < counterList.size(); ++i)
		counterList[i]->writeElement(xml);
	for (int i = 0; i < arrowList.size(); ++i)
		arrowList[i]->writeElement(xml);
	xml->writeEndElement();
}

bool DeckList_File::readElement(QXmlStreamReader *xml)
{
	if (xml->isEndElement())
		return true;
	else
		return false;
}

void DeckList_File::writeElement(QXmlStreamWriter *xml)
{
	xml->writeStartElement("file");
	xml->writeAttribute("name", name);
	xml->writeAttribute("id", QString::number(id));
	xml->writeAttribute("upload_time", QString::number(uploadTime.toTime_t()));
	xml->writeEndElement();
}

DeckList_Directory::~DeckList_Directory()
{
	for (int i = 0; i < size(); ++i)
		delete at(i);
}

bool DeckList_Directory::readElement(QXmlStreamReader *xml)
{
	if (currentItem) {
		if (currentItem->readElement(xml))
			currentItem = 0;
		return false;
	}
	if (xml->isStartElement() && (xml->name() == "directory")) {
		DeckList_Directory *newItem = new DeckList_Directory(xml->attributes().value("name").toString());
		append(newItem);
		currentItem = newItem;
	} else if (xml->isStartElement() && (xml->name() == "file")) {
		DeckList_File *newItem = new DeckList_File(xml->attributes().value("name").toString(), xml->attributes().value("id").toString().toInt(), QDateTime::fromTime_t(xml->attributes().value("upload_time").toString().toUInt()));
		append(newItem);
		currentItem = newItem;
	} else if (xml->isEndElement() && (xml->name() == "directory"))
		return true;

	return false;
}

void DeckList_Directory::writeElement(QXmlStreamWriter *xml)
{
	xml->writeStartElement("directory");
	xml->writeAttribute("name", name);
	for (int i = 0; i < size(); ++i)
		at(i)->writeElement(xml);
	xml->writeEndElement();
}
