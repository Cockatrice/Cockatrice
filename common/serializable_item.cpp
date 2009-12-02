#include "serializable_item.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

QHash<QString, SerializableItem::NewItemFunction> SerializableItem::itemNameHash;

SerializableItem *SerializableItem::getNewItem(const QString &name)
{
	if (!itemNameHash.contains(name))
		return 0;
	return itemNameHash.value(name)();
}

void SerializableItem::registerSerializableItem(const QString &name, NewItemFunction func)
{
	itemNameHash.insert(name, func);
}

bool SerializableItem::readElement(QXmlStreamReader *xml)
{
	if (xml->isEndElement() && (xml->name() == itemType))
		return true;
	return false;
}

void SerializableItem::write(QXmlStreamWriter *xml)
{
	xml->writeStartElement(itemType);
	if (!itemSubType.isEmpty())
		xml->writeAttribute("type", itemSubType);
	writeElement(xml);
	xml->writeEndElement();
}

SerializableItem_Map::~SerializableItem_Map()
{
	QMapIterator<QString, SerializableItem *> mapIterator(itemMap);
	while (mapIterator.hasNext())
		delete mapIterator.next().value();
	for (int i = 0; i < itemList.size(); ++i)
		delete itemList[i];
}

bool SerializableItem_Map::readElement(QXmlStreamReader *xml)
{
	if (currentItem) {
		if (currentItem->readElement(xml))
			currentItem = 0;
		return false;
	} else if (firstItem)
		firstItem = false;
	else if (xml->isEndElement() && (xml->name() == itemType))
		extractData();
	else if (xml->isStartElement()) {
		QString childName = xml->name().toString();
		QString childSubType = xml->attributes().value("type").toString();
		currentItem = itemMap.value(childName);
		if (!currentItem) {
			currentItem = getNewItem(childName + childSubType);
			itemList.append(currentItem);
			if (!currentItem)
				currentItem = new SerializableItem_Invalid(childName);
		}
		if (currentItem->readElement(xml))
			currentItem = 0;
	}
	return SerializableItem::readElement(xml);
}

void SerializableItem_Map::writeElement(QXmlStreamWriter *xml)
{
	QMapIterator<QString, SerializableItem *> mapIterator(itemMap);
	while (mapIterator.hasNext())
		mapIterator.next().value()->write(xml);
	for (int i = 0; i < itemList.size(); ++i)
		itemList[i]->write(xml);
}

bool SerializableItem_String::readElement(QXmlStreamReader *xml)
{
	if (xml->isCharacters() && !xml->isWhitespace())
		data = xml->text().toString();
	return SerializableItem::readElement(xml);
}

void SerializableItem_String::writeElement(QXmlStreamWriter *xml)
{
	xml->writeCharacters(data);
}

bool SerializableItem_Int::readElement(QXmlStreamReader *xml)
{
	if (xml->isCharacters() && !xml->isWhitespace()) {
		bool ok;
		data = xml->text().toString().toInt(&ok);
		if (!ok)
			data = -1;
	}
	return SerializableItem::readElement(xml);
}

void SerializableItem_Int::writeElement(QXmlStreamWriter *xml)
{
	xml->writeCharacters(QString::number(data));
}

bool SerializableItem_Bool::readElement(QXmlStreamReader *xml)
{
	if (xml->isCharacters() && !xml->isWhitespace())
		data = xml->text().toString() == "1";
	return SerializableItem::readElement(xml);
}

void SerializableItem_Bool::writeElement(QXmlStreamWriter *xml)
{
	xml->writeCharacters(data ? "1" : "0");
}

int SerializableItem_Color::colorToInt(const QColor &color) const
{
	return color.red() * 65536 + color.green() * 256 + color.blue();
}

QColor SerializableItem_Color::colorFromInt(int colorValue) const
{
	return QColor(colorValue / 65536, (colorValue % 65536) / 256, colorValue % 256);
}

bool SerializableItem_Color::readElement(QXmlStreamReader *xml)
{
	if (xml->isCharacters() && !xml->isWhitespace()) {
		bool ok;
		int colorValue = xml->text().toString().toInt(&ok);
		data = ok ? colorFromInt(colorValue) : Qt::black;
	}
	return SerializableItem::readElement(xml);
}

void SerializableItem_Color::writeElement(QXmlStreamWriter *xml)
{
	xml->writeCharacters(QString::number(colorToInt(data)));
}

bool SerializableItem_DateTime::readElement(QXmlStreamReader *xml)
{
	if (xml->isCharacters() && !xml->isWhitespace()) {
		bool ok;
		unsigned int dateTimeValue = xml->text().toString().toUInt(&ok);
		data = ok ? QDateTime::fromTime_t(dateTimeValue) : QDateTime();
	}
	return SerializableItem::readElement(xml);
}

void SerializableItem_DateTime::writeElement(QXmlStreamWriter *xml)
{
	xml->writeCharacters(QString::number(data.toTime_t()));
}
