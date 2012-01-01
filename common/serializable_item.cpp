#include "serializable_item.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QBuffer>

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

bool SerializableItem::read(QXmlStreamReader *xml)
{
	if (!compressed)
		return readElement(xml);
	if (xml->isEndElement() && (xml->name() == itemType)) {
		QByteArray uncompressedData = "<d>" + qUncompress(QByteArray::fromBase64(compressedData)) + "</d>";
		compressedData.clear();
		QBuffer compressedBuffer(&uncompressedData);
		compressedBuffer.open(QIODevice::ReadOnly);
		QXmlStreamReader *xml2 = new QXmlStreamReader(&compressedBuffer);
		while (!xml2->atEnd())  {
			xml2->readNext();
			if (xml2->name() == "d")
				continue;
			readElement(xml2);
		}
		delete xml2;
		compressedBuffer.close();
		
		return readElement(xml);
	} else {
		compressedData.append(xml->text().toString());
		return false;
	}
}

bool SerializableItem::readElement(QXmlStreamReader *xml)
{
	if (xml->isEndElement() && (xml->name() == itemType))
		return true;
	return false;
}

void SerializableItem::write(QXmlStreamWriter *xml)
{
	if (isEmpty())
		return;
	
	xml->writeStartElement(itemType);
	if (!itemSubType.isEmpty())
		xml->writeAttribute("type", itemSubType);
	if (compressed) {
		xml->writeAttribute("comp", "1");

		QBuffer compressBuffer;
		compressBuffer.open(QIODevice::WriteOnly);
		QXmlStreamWriter *xml2 = new QXmlStreamWriter(&compressBuffer);
		writeElement(xml2);
		delete xml2;
		compressBuffer.close();

		xml->writeCharacters(qCompress(compressBuffer.data()).toBase64());
	} else
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
		if (currentItem->read(xml))
			currentItem = 0;
		return false;
	} else if (firstItem)
		firstItem = false;
	else if (xml->isEndElement() && (xml->name() == itemType))
		extractData();
	else if (xml->isStartElement()) {
		QString childName = xml->name().toString();
		QString childSubType = xml->attributes().value("type").toString();
		bool childCompressed = xml->attributes().value("comp").toString().toInt() == 1;
		currentItem = itemMap.value(childName);
		if (!currentItem) {
			currentItem = getNewItem(childName + childSubType);
			itemList.append(currentItem);
			if (!currentItem)
				currentItem = new SerializableItem_Invalid(childName);
		}
		currentItem->setCompressed(childCompressed);
		if (currentItem->read(xml))
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
	// This function is sometimes called multiple times if there are
	// entities in the strings, so we have to make sure the data is
	// not overwritten but appended to.
	if (xml->isCharacters() && !xml->isWhitespace())
		data.append(xml->text().toString());
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

bool SerializableItem_ByteArray::readElement(QXmlStreamReader *xml)
{
	if (xml->isCharacters() && !xml->isWhitespace())
		data = qUncompress(QByteArray::fromBase64(xml->text().toString().toAscii()));
	
	return SerializableItem::readElement(xml);
}

void SerializableItem_ByteArray::writeElement(QXmlStreamWriter *xml)
{
	xml->writeCharacters(QString(qCompress(data).toBase64()));
}
