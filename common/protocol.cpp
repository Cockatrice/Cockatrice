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

bool ProtocolItem::read(QXmlStreamReader *xml)
{
	while (!xml->atEnd()) {
		xml->readNext();
		if (readElement(xml))
			continue;
		if (xml->isEndElement()) {
			if (xml->name() == getItemType()) {
				extractParameters();
				return true;
			} else {
				QString tagName = xml->name().toString();
				if (parameters.contains(tagName))
					parameters[tagName] = currentElementText;
				currentElementText.clear();
			}
		} else if (xml->isCharacters() && !xml->isWhitespace())
			currentElementText = xml->text().toString();
	}
	return false;
}

void ProtocolItem::write(QXmlStreamWriter *xml)
{
	xml->writeStartElement(getItemType());
	if (!itemName.isEmpty())
		xml->writeAttribute("name", itemName);
	
	QMapIterator<QString, QString> i(parameters);
	while (i.hasNext()) {
		i.next();
		xml->writeTextElement(i.key(), i.value());
	}
	
	writeElement(xml);
	
	xml->writeEndElement();
}

ProtocolItem *ProtocolItem::getNewItem(const QString &name)
{
	if (!itemNameHash.contains(name))
		return 0;
	return itemNameHash.value(name)();
}

void ProtocolItem::initializeHash()
{
	if (!itemNameHash.isEmpty())
		return;
	
	initializeHashAuto();
	
	itemNameHash.insert("cmddeck_upload", Command_DeckUpload::newItem);
	
	itemNameHash.insert("resp", ProtocolResponse::newItem);
	ProtocolResponse::initializeHash();
	itemNameHash.insert("respdeck_list", Response_DeckList::newItem);
	
	itemNameHash.insert("generic_eventlist_games", Event_ListGames::newItem);
	itemNameHash.insert("generic_eventlist_chat_channels", Event_ListChatChannels::newItem);
	itemNameHash.insert("chat_eventchat_list_players", Event_ChatListPlayers::newItem);
}

int Command::lastCmdId = 0;

Command::Command(const QString &_itemName, int _cmdId)
	: ProtocolItem(_itemName), cmdId(_cmdId), ticks(0)
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

void Command::processResponse(ProtocolResponse *response)
{
	emit finished(response);
	emit finished(response->getResponseCode());
}

QHash<QString, ResponseCode> ProtocolResponse::responseHash;

ProtocolResponse::ProtocolResponse(int _cmdId, ResponseCode _responseCode, const QString &_itemName)
	: ProtocolItem(_itemName), cmdId(_cmdId), responseCode(_responseCode)
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
	responseHash.insert("invalid_command", RespInvalidCommand);
	responseHash.insert("name_not_found", RespNameNotFound);
	responseHash.insert("login_needed", RespLoginNeeded);
	responseHash.insert("context_error", RespContextError);
	responseHash.insert("wrong_password", RespWrongPassword);
	responseHash.insert("spectators_not_allowed", RespSpectatorsNotAllowed);
}

bool Response_DeckList::File::readElement(QXmlStreamReader *xml)
{
	if (xml->isEndElement())
		return true;
	else
		return false;
}

void Response_DeckList::File::writeElement(QXmlStreamWriter *xml)
{
	xml->writeStartElement("file");
	xml->writeAttribute("name", name);
	xml->writeAttribute("id", QString::number(id));
	xml->writeEndElement();
}

Response_DeckList::Directory::~Directory()
{
	for (int i = 0; i < size(); ++i)
		delete at(i);
}

bool Response_DeckList::Directory::readElement(QXmlStreamReader *xml)
{
	if (currentItem) {
		if (currentItem->readElement(xml))
			currentItem = 0;
		return false;
	}
	if (xml->isStartElement() && (xml->name() == "directory")) {
		currentItem = new Directory(xml->attributes().value("name").toString());
		append(currentItem);
	} else if (xml->isStartElement() && (xml->name() == "file")) {
		currentItem = new File(xml->attributes().value("name").toString(), xml->attributes().value("id").toString().toInt());
		append(currentItem);
	} else if (xml->isEndElement() && (xml->name() == "directory"))
		return true;
	
	return false;
}

void Response_DeckList::Directory::writeElement(QXmlStreamWriter *xml)
{
	xml->writeStartElement("directory");
	xml->writeAttribute("name", name);
	for (int i = 0; i < size(); ++i)
		at(i)->writeElement(xml);
	xml->writeEndElement();
}

Response_DeckList::Response_DeckList(int _cmdId, ResponseCode _responseCode, Directory *_root)
	: ProtocolResponse(_cmdId, _responseCode, "deck_list"), root(_root)
{
}

Response_DeckList::~Response_DeckList()
{
	delete root;
}

bool Response_DeckList::readElement(QXmlStreamReader *xml)
{
	if (!root) {
		if (xml->isStartElement() && (xml->name() == "directory"))
			root = new Directory;
		return false;
	}
	
	return !root->readElement(xml);
}

void Response_DeckList::writeElement(QXmlStreamWriter *xml)
{
	root->writeElement(xml);
}

GenericEvent::GenericEvent(const QString &_eventName)
	: ProtocolItem(_eventName)
{
}

void GameEvent::extractParameters()
{
	bool ok;
	gameId = parameters["game_id"].toInt(&ok);
	if (!ok)
		gameId = -1;
	playerId = parameters["player_id"].toInt(&ok);
	if (!ok)
		playerId = -1;
}

GameEvent::GameEvent(const QString &_eventName, int _gameId, int _playerId)
	: ProtocolItem(_eventName), gameId(_gameId), playerId(_playerId)
{
	setParameter("game_id", gameId);
	setParameter("player_id", playerId);
}

void ChatEvent::extractParameters()
{
	channel = parameters["channel"];
}

ChatEvent::ChatEvent(const QString &_eventName, const QString &_channel)
	: ProtocolItem(_eventName), channel(_channel)
{
	setParameter("channel", channel);
}

bool Event_ListChatChannels::readElement(QXmlStreamReader *xml)
{
	if (xml->isStartElement() && (xml->name() == "channel")) {
		channelList.append(ServerChatChannelInfo(
			xml->attributes().value("name").toString(),
			xml->attributes().value("description").toString(),
			xml->attributes().value("player_count").toString().toInt(),
			xml->attributes().value("auto_join").toString().toInt()
		));
		return true;
	}
	return false;
}

void Event_ListChatChannels::writeElement(QXmlStreamWriter *xml)
{
	for (int i = 0; i < channelList.size(); ++i) {
		xml->writeStartElement("channel");
		xml->writeAttribute("name", channelList[i].getName());
		xml->writeAttribute("description", channelList[i].getDescription());
		xml->writeAttribute("player_count", QString::number(channelList[i].getPlayerCount()));
		xml->writeAttribute("auto_join", channelList[i].getAutoJoin() ? "1" : "0");
		xml->writeEndElement();
	}
}

bool Event_ChatListPlayers::readElement(QXmlStreamReader *xml)
{
	if (xml->isStartElement() && ((xml->name() == "player"))) {
		playerList.append(ServerPlayerInfo(
			xml->attributes().value("name").toString()
		));
		return true;
	}
	return false;
}

void Event_ChatListPlayers::writeElement(QXmlStreamWriter *xml)
{
	for (int i = 0; i < playerList.size(); ++i) {
		xml->writeStartElement("player");
		xml->writeAttribute("name", playerList[i].getName());
		xml->writeEndElement();
	}
}

bool Event_ListGames::readElement(QXmlStreamReader *xml)
{
	if (xml->isStartElement() && (xml->name() == "game")) {
		gameList.append(ServerGameInfo(
			xml->attributes().value("id").toString().toInt(),
			xml->attributes().value("description").toString(),
			xml->attributes().value("has_password").toString().toInt(),
			xml->attributes().value("player_count").toString().toInt(),
			xml->attributes().value("max_players").toString().toInt(),
			xml->attributes().value("creator").toString(),
			xml->attributes().value("spectators_allowed").toString().toInt(),
			xml->attributes().value("spectator_count").toString().toInt()
		));
		return true;
	}
	return false;
}

void Event_ListGames::writeElement(QXmlStreamWriter *xml)
{
	for (int i = 0; i < gameList.size(); ++i) {
		xml->writeStartElement("game");
		xml->writeAttribute("id", QString::number(gameList[i].getGameId()));
		xml->writeAttribute("description", gameList[i].getDescription());
		xml->writeAttribute("has_password", gameList[i].getHasPassword() ? "1" : "0");
		xml->writeAttribute("player_count", QString::number(gameList[i].getPlayerCount()));
		xml->writeAttribute("max_players", QString::number(gameList[i].getMaxPlayers()));
		xml->writeAttribute("creator", gameList[i].getCreatorName());
		xml->writeAttribute("spectators_allowed", gameList[i].getSpectatorsAllowed() ? "1" : "0");
		xml->writeAttribute("spectator_count", QString::number(gameList[i].getSpectatorCount()));
		xml->writeEndElement();
	}
}
