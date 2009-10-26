#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>
#include "protocol.h"
#include "protocol_items.h"

QHash<QString, ProtocolItem::NewItemFunction> ProtocolItem::itemNameHash;

ProtocolItem::ProtocolItem(const QString &_itemName)
	: itemName(_itemName)
{
}

bool ProtocolItem::read(QXmlStreamReader &xml)
{
	while (!xml.atEnd()) {
		xml.readNext();
		if (xml.isStartElement()) {
			qDebug() << "startElement: " << xml.name().toString();
		} else if (xml.isEndElement()) {
			qDebug() << "endElement: " << xml.name().toString();
			if (xml.name() == getItemType()) {
				extractParameters();
				qDebug() << "FERTIG";
				deleteLater();
				return true;
			} else {
				QString tagName = xml.name().toString();
				if (!parameters.contains(tagName))
					qDebug() << "unrecognized attribute";
				else
					parameters[tagName] = currentElementText;
			}
		} else if (xml.isCharacters() && !xml.isWhitespace()) {
			currentElementText = xml.text().toString();
			qDebug() << "text: " << currentElementText;
		}
	}
	return false;
}

void ProtocolItem::write(QXmlStreamWriter &xml)
{
	xml.writeStartElement(getItemType());
	if (!itemName.isEmpty())
		xml.writeAttribute("name", itemName);
	
	QMapIterator<QString, QString> i(parameters);
	while (i.hasNext()) {
		i.next();
		xml.writeTextElement(i.key(), i.value());
	}
	
	xml.writeEndElement();
}

ProtocolItem *ProtocolItem::getNewItem(const QString &name)
{
	if (!itemNameHash.contains(name))
		return 0;
	return itemNameHash.value(name)();
}

void ProtocolItem::initializeHash()
{
	initializeHashAuto();
	itemNameHash.insert("resp", ProtocolResponse::newItem);
	ProtocolResponse::initializeHash();
}

int Command::lastCmdId = 0;

Command::Command(const QString &_itemName, int _cmdId)
	: ProtocolItem(_itemName), cmdId(_cmdId)
{
	if (cmdId == -1)
		cmdId = lastCmdId++;
	setParameter("cmd_id", cmdId);
}

void Command::extractParameters()
{
	bool ok;
	cmdId = parameters["cmd_id"].toInt(&ok);
	if (!ok)
		cmdId = -1;
}

QHash<QString, ProtocolResponse::ResponseCode> ProtocolResponse::responseHash;

ProtocolResponse::ProtocolResponse(int _cmdId, ResponseCode _responseCode)
	: ProtocolItem(QString()), cmdId(_cmdId), responseCode(_responseCode)
{
	setParameter("cmd_id", cmdId);
	setParameter("response_code", responseHash.key(responseCode));
}

void ProtocolResponse::extractParameters()
{
	bool ok;
	cmdId = parameters["cmd_id"].toInt(&ok);
	if (!ok)
		cmdId = -1;
	
	responseCode = responseHash.value(parameters["response_code"], RespOk);
}

void ProtocolResponse::initializeHash()
{
	responseHash.insert("ok", RespOk);
	responseHash.insert("name_not_found", RespNameNotFound);
	responseHash.insert("login_needed", RespLoginNeeded);
	responseHash.insert("context_error", RespContextError);
	responseHash.insert("wrong_password", RespWrongPassword);
	responseHash.insert("spectators_not_allowed", RespSpectatorsNotAllowed);
}

void GameEvent::extractParameters()
{
	bool ok;
	gameId = parameters["game_id"].toInt(&ok);
	if (!ok)
		gameId = -1;
	isPublic = parameters["is_public"].toInt();
	playerId = parameters["player_id"].toInt(&ok);
	if (!ok)
		playerId = -1;
}

GameEvent::GameEvent(const QString &_eventName, int _gameId, bool _isPublic, int _playerId)
	: ProtocolItem(_eventName), gameId(_gameId), isPublic(_isPublic), playerId(_playerId)
{
	setParameter("game_id", gameId);
	setParameter("is_public", isPublic);
	setParameter("player_id", playerId);
}
