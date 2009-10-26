#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>
#include <QMap>
#include <QHash>
#include <QObject>
#include <QDebug>

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
	ProtocolItem(const QString &_itemName);
	static void initializeHash();
	static ProtocolItem *getNewItem(const QString &name);
	virtual bool read(QXmlStreamReader &xml);
	virtual void write(QXmlStreamWriter &xml);
};

class Command : public ProtocolItem {
private:
	int cmdId;
	static int lastCmdId;
protected:
	QString getItemType() const { return "cmd"; }
	void extractParameters();
public:
	Command(const QString &_itemName, int _cmdId = -1);
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
	enum ResponseCode { RespOk, RespNameNotFound, RespLoginNeeded, RespContextError, RespWrongPassword, RespSpectatorsNotAllowed };
private:
	int cmdId;
	ResponseCode responseCode;
	static QHash<QString, ResponseCode> responseHash;
protected:
	QString getItemType() const { return "resp"; }
	void extractParameters();
public:
	ProtocolResponse(int _cmdId = -1, ResponseCode _responseCode = RespOk);
	static void initializeHash();
	static ProtocolItem *newItem() { return new ProtocolResponse; }
};

class GameEvent : public ProtocolItem {
	Q_OBJECT
private:
	int gameId;
	bool isPublic;
	int playerId;
protected:
	QString getItemType() const { return "game_event"; }
	void extractParameters();
public:
	GameEvent(const QString &_eventName, int _gameId, bool _isPublic, int _playerId);
};

#endif
