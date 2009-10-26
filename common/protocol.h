#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>
#include <QMap>
#include <QHash>
#include <QObject>
#include <QDebug>

class QXmlStreamReader;
class QXmlStreamWriter;

class Command : public QObject {
	Q_OBJECT
protected:
	typedef Command *(*NewCommandFunction)();
	static QHash<QString, NewCommandFunction> commandHash;
	
	QString cmdName;
	QMap<QString, QString> parameters;
	QString currentElementText;
	void setParameter(const QString &name, const QString &value) { parameters[name] = value; }
	void setParameter(const QString &name, bool value) { parameters[name] = (value ? "1" : "0"); }
	void setParameter(const QString &name, int value) { parameters[name] = QString::number(value); }
	virtual void extractParameters() { };
public:
	Command(const QString &_cmdName);
	static void initializeHash();
	static Command *getNewCommand(const QString &name);
	virtual bool read(QXmlStreamReader &xml);
	virtual void write(QXmlStreamWriter &xml);
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

#endif
