#ifndef SERIALIZABLE_ITEM_H
#define SERIALIZABLE_ITEM_H

#include <QtCore/QXmlStreamWriter>
#include <QObject>
#include <QMap>
#include <QList>
#include <QHash>
#include <QDateTime>
#include <QStringList>
#include "color.h"

class QXmlStreamReader;
class QXmlStreamWriter;

class SerializableItem : public QObject {
	Q_OBJECT
protected:
	typedef SerializableItem *(*NewItemFunction)();
	static QHash<QString, NewItemFunction> itemNameHash;
	
	QString itemType, itemSubType;
	bool firstItem;
public:
	SerializableItem(const QString &_itemType, const QString &_itemSubType = QString())
		: QObject(), itemType(_itemType), itemSubType(_itemSubType), firstItem(true) { }
	static void registerSerializableItem(const QString &name, NewItemFunction func);
	static SerializableItem *getNewItem(const QString &name);
	const QString &getItemType() const { return itemType; }
	const QString &getItemSubType() const { return itemSubType; }
	virtual bool readElement(QXmlStreamReader *xml);
	virtual void writeElement(QXmlStreamWriter *xml) = 0;
	virtual bool isEmpty() const = 0;
	void write(QXmlStreamWriter *xml);
};

class SerializableItem_Invalid : public SerializableItem {
public:
	SerializableItem_Invalid(const QString &_itemType) : SerializableItem(_itemType) { }
	void writeElement(QXmlStreamWriter * /*xml*/) { }
	bool isEmpty() const { return true; }
};

class SerializableItem_Map : public SerializableItem {
private:
	SerializableItem *currentItem;
protected:
	QMap<QString, SerializableItem *> itemMap;
	QList<SerializableItem *> itemList;
	virtual void extractData() { }
	void insertItem(SerializableItem *item)
	{
		itemMap.insert(item->getItemType(), item);
	}
	template<class T> QList<T> typecastItemList() const
	{
		QList<T> result;
		for (int i = 0; i < itemList.size(); ++i) {
			T item = dynamic_cast<T>(itemList[i]);
			if (item)
				result.append(item);
		}
		return result;
	}
public:
	SerializableItem_Map(const QString &_itemType, const QString &_itemSubType = QString())
		: SerializableItem(_itemType, _itemSubType), currentItem(0)
	{
	}
	~SerializableItem_Map();
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
	bool isEmpty() const { return itemMap.isEmpty() && itemList.isEmpty(); }
	void appendItem(SerializableItem *item) { itemList.append(item); }
};

class SerializableItem_String : public SerializableItem {
private:
	QString data;
protected:
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
public:
	SerializableItem_String(const QString &_itemType, const QString &_data = QString())
		: SerializableItem(_itemType), data(_data) { }
	const QString &getData() { return data; }
	void setData(const QString &_data) { data = _data; }
	bool isEmpty() const { return data.isEmpty(); }
};

class SerializableItem_Int : public SerializableItem {
private:
	int data;
protected:
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
public:
	SerializableItem_Int(const QString &_itemType, int _data = -1)
		: SerializableItem(_itemType), data(_data) { }
	int getData() { return data; }
	void setData(int _data) { data = _data; }
	bool isEmpty() const { return data == -1; }
};

class SerializableItem_Bool : public SerializableItem {
private:
	bool data;
protected:
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
public:
	SerializableItem_Bool(const QString &_itemType, bool _data = false)
		: SerializableItem(_itemType), data(_data) { }
	bool getData() { return data; }
	void setData(bool _data) { data = _data; }
	bool isEmpty() const { return data == false; }
};

class SerializableItem_Color : public SerializableItem {
private:
	Color data;
protected:
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
public:
	SerializableItem_Color(const QString &_itemType, const Color &_data = Color())
		: SerializableItem(_itemType), data(_data) { }
	const Color &getData() { return data; }
	void setData(const Color &_data) { data = _data; }
	bool isEmpty() const { return data.getValue() == 0; }
};

class SerializableItem_DateTime : public SerializableItem {
private:
	QDateTime data;
protected:
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
public:
	SerializableItem_DateTime(const QString &_itemType, const QDateTime &_data = QDateTime())
		: SerializableItem(_itemType), data(_data) { }
	const QDateTime &getData() { return data; }
	void setData(const QDateTime &_data) { data = _data; }
	bool isEmpty() const { return data == QDateTime(); }
};

class SerializableItem_ByteArray : public SerializableItem {
private:
	QByteArray data;
protected:
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
public:
	SerializableItem_ByteArray(const QString &_itemType, const QByteArray &_data = QByteArray())
		: SerializableItem(_itemType), data(_data) { }
	const QByteArray &getData() { return data; }
	void setData(const QByteArray &_data) { data = _data; }
	bool isEmpty() const { return data.isEmpty(); }
};

#endif
