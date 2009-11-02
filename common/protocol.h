#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>
#include <QMap>
#include <QHash>
#include <QObject>
#include <QDebug>
#include "protocol_item_ids.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class QXmlStreamAttributes;

class ProtocolItem : public QObject {
	Q_OBJECT
protected:
	typedef ProtocolItem *(*NewItemFunction)();
	static QHash<QString, NewItemFunction> itemNameHash;
	
	QString itemName;
	QMap<QString, QString> parameters;
	QString currentElementText;
	void setParameter(const QString &name, const QString &value) { parameters[name] = value; }
	void setParameter(const QString &name, bool value) { parameters[name] = (value ? "1" : "0"); }
	void setParameter(const QString &name, int value) { parameters[name] = QString::number(value); }
	virtual void extractParameters() { };
	virtual QString getItemType() const = 0;
private:
	static void initializeHashAuto();
public:
	static const int protocolVersion = 4;
	virtual int getItemId() const = 0;
	ProtocolItem(const QString &_itemName);
	static void initializeHash();
	static ProtocolItem *getNewItem(const QString &name);
	virtual bool read(QXmlStreamReader *xml);
	virtual void write(QXmlStreamWriter *xml);
};

class Command : public ProtocolItem {
	Q_OBJECT
private:
	int cmdId;
	static int lastCmdId;
protected:
	QString getItemType() const { return "cmd"; }
	void extractParameters();
public:
	Command(const QString &_itemName = QString(), int _cmdId = -1);
	int getCmdId() const { return cmdId; }
};

class InvalidCommand : public Command {
	Q_OBJECT
public:
	InvalidCommand() : Command() { }
	int getItemId() const { return ItemId_Other; }
};

class ChatCommand : public Command {
	Q_OBJECT
private:
	QString channel;
protected:
	void extractParameters()
	{
		channel = parameters["channel"];
	}
public:
	ChatCommand(const QString &_cmdName, const QString &_channel)
		: Command(_cmdName), channel(_channel)
	{
		setParameter("channel", channel);
	}
	QString getChannel() const { return channel; }
};

class GameCommand : public Command {
	Q_OBJECT
private:
	int gameId;
protected:
	void extractParameters()
	{
		gameId = parameters["game_id"].toInt();
	}
public:
	GameCommand(const QString &_cmdName, int _gameId)
		: Command(_cmdName), gameId(_gameId)
	{
		setParameter("game_id", gameId);
	}
	int getGameId() const { return gameId; }
};

class ProtocolResponse : public ProtocolItem {
	Q_OBJECT
public:
	enum ResponseCode { RespNothing, RespOk, RespInvalidCommand, RespNameNotFound, RespLoginNeeded, RespContextError, RespWrongPassword, RespSpectatorsNotAllowed };
private:
	int cmdId;
	ResponseCode responseCode;
	static QHash<QString, ResponseCode> responseHash;
protected:
	QString getItemType() const { return "resp"; }
	void extractParameters();
public:
	ProtocolResponse(int _cmdId = -1, ResponseCode _responseCode = RespOk);
	int getItemId() const { return ItemId_Other; }
	static void initializeHash();
	static ProtocolItem *newItem() { return new ProtocolResponse; }
};

class GenericEvent : public ProtocolItem {
	Q_OBJECT
protected:
	QString getItemType() const { return "generic_event"; }
public:
	GenericEvent(const QString &_eventName);
};

class GameEvent : public ProtocolItem {
	Q_OBJECT
private:
	int gameId;
	int playerId;
protected:
	QString getItemType() const { return "game_event"; }
	void extractParameters();
public:
	GameEvent(const QString &_eventName, int _gameId, int _playerId);
};

class ChatEvent : public ProtocolItem {
	Q_OBJECT
private:
	QString channel;
protected:
	QString getItemType() const { return "chat_event"; }
	void extractParameters();
public:
	ChatEvent(const QString &_eventName, const QString &_channel);
};

#endif
