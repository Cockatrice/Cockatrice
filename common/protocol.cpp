#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>
#include "protocol.h"
#include "protocol_items.h"
#include "decklist.h"

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
	itemNameHash.insert("cmddeck_select", Command_DeckSelect::newItem);
	
	itemNameHash.insert("resp", ProtocolResponse::newItem);
	ProtocolResponse::initializeHash();
	itemNameHash.insert("respdeck_list", Response_DeckList::newItem);
	itemNameHash.insert("respdeck_download", Response_DeckDownload::newItem);
	itemNameHash.insert("respdeck_upload", Response_DeckUpload::newItem);
	
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

Command_DeckUpload::Command_DeckUpload(DeckList *_deck, const QString &_path)
	: Command("deck_upload"), deck(_deck), path(_path), readFinished(false)
{
	setParameter("path", path);
}

void Command_DeckUpload::extractParameters()
{
	Command::extractParameters();
	
	path = parameters["path"];
}

bool Command_DeckUpload::readElement(QXmlStreamReader *xml)
{
	if (readFinished)
		return false;
	
	if (!deck) {
		if (xml->isStartElement() && (xml->name() == "cockatrice_deck")) {
			deck = new DeckList;
			return true;
		}
		return false;
	}
	
	if (deck->readElement(xml))
		readFinished = true;
	return true;
}

void Command_DeckUpload::writeElement(QXmlStreamWriter *xml)
{
	if (deck)
		deck->writeElement(xml);
}

Command_DeckSelect::Command_DeckSelect(int _gameId, DeckList *_deck, int _deckId)
	: GameCommand("deck_select", _gameId), deck(_deck), deckId(_deckId), readFinished(false)
{
	setParameter("deck_id", _deckId);
}

void Command_DeckSelect::extractParameters()
{
	GameCommand::extractParameters();
	
	bool ok;
	deckId = parameters["deck_id"].toInt(&ok);
	if (!ok)
		deckId = -1;
}

bool Command_DeckSelect::readElement(QXmlStreamReader *xml)
{
	if (readFinished)
		return false;
	
	if (!deck) {
		if (xml->isStartElement() && (xml->name() == "cockatrice_deck")) {
			deck = new DeckList;
			return true;
		}
		return false;
	}
	
	if (deck->readElement(xml))
		readFinished = true;
	return true;
}

void Command_DeckSelect::writeElement(QXmlStreamWriter *xml)
{
	if (deck)
		deck->writeElement(xml);
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
		currentItem = new DeckList_Directory(xml->attributes().value("name").toString());
		append(currentItem);
	} else if (xml->isStartElement() && (xml->name() == "file")) {
		currentItem = new DeckList_File(xml->attributes().value("name").toString(), xml->attributes().value("id").toString().toInt(), QDateTime::fromTime_t(xml->attributes().value("upload_time").toString().toUInt()));
		append(currentItem);
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

Response_DeckList::Response_DeckList(int _cmdId, ResponseCode _responseCode, DeckList_Directory *_root)
	: ProtocolResponse(_cmdId, _responseCode, "deck_list"), root(_root), readFinished(false)
{
}

Response_DeckList::~Response_DeckList()
{
	delete root;
}

bool Response_DeckList::readElement(QXmlStreamReader *xml)
{
	if (readFinished)
		return false;
	
	if (!root) {
		if (xml->isStartElement() && (xml->name() == "directory")) {
			root = new DeckList_Directory;
			return true;
		}
		return false;
	}
	
	if (root->readElement(xml))
		readFinished = true;
	return true;
}

void Response_DeckList::writeElement(QXmlStreamWriter *xml)
{
	root->writeElement(xml);
}

Response_DeckDownload::Response_DeckDownload(int _cmdId, ResponseCode _responseCode, DeckList *_deck)
	: ProtocolResponse(_cmdId, _responseCode, "deck_download"), deck(_deck), readFinished(false)
{
}

bool Response_DeckDownload::readElement(QXmlStreamReader *xml)
{
	if (readFinished)
		return false;
	
	if (!deck) {
		if (xml->isStartElement() && (xml->name() == "cockatrice_deck")) {
			deck = new DeckList;
			return true;
		}
		return false;
	}
	
	if (deck->readElement(xml))
		readFinished = true;
	return true;
}

void Response_DeckDownload::writeElement(QXmlStreamWriter *xml)
{
	if (deck)
		deck->writeElement(xml);
}

Response_DeckUpload::Response_DeckUpload(int _cmdId, ResponseCode _responseCode, DeckList_File *_file)
	: ProtocolResponse(_cmdId, _responseCode, "deck_upload"), file(_file), readFinished(false)
{
}

Response_DeckUpload::~Response_DeckUpload()
{
	delete file;
}

bool Response_DeckUpload::readElement(QXmlStreamReader *xml)
{
	if (readFinished)
		return false;
	
	if (!file) {
		if (xml->isStartElement() && (xml->name() == "file")) {
			file = new DeckList_File(xml->attributes().value("name").toString(), xml->attributes().value("id").toString().toInt(), QDateTime::fromTime_t(xml->attributes().value("upload_time").toString().toUInt()));
			return true;
		}
		return false;
	}
	
	if (file->readElement(xml))
		readFinished = true;
	return true;
}

void Response_DeckUpload::writeElement(QXmlStreamWriter *xml)
{
	if (file)
		file->writeElement(xml);
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
