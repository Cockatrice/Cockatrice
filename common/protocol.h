#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>
#include <QMap>
#include <QObject>

class QXmlStreamReader;
class QXmlStreamWriter;

class Command : public QObject {
	Q_OBJECT
protected:
	QString cmdName;
	QMap<QString, QString> parameters;
	QString currentElementText;
public:
	Command(const QString &_cmdName);
	virtual bool read(QXmlStreamReader &xml);
	virtual void write(QXmlStreamWriter &xml);
	void validateParameters();
};

class Command_Ping : public Command {
public:
	Command_Ping() : Command("ping") { }
};
class Command_ChatListChannels : public Command {
public:
	Command_ChatListChannels() : Command("chat_list_channels") { }
};
class Command_ChatJoinChannel : public Command {
private:
	QString channel;
public:
	Command_ChatJoinChannel(const QString &_channel = QString()) : Command("chat_join_channel"), channel(_channel)
	{
		parameters.insert("channel", channel);
	}
};
class Command_ChatLeaveChannel : public Command {
private:
	QString channel;
public:
	Command_ChatLeaveChannel(const QString &_channel = QString()) : Command("chat_leave_channel"), channel(_channel)
	{
		parameters.insert("channel", channel);
	}
};
class Command_ChatSay : public Command {
private:
	QString channel;
	QString message;
public:
	Command_ChatSay(const QString &_channel = QString(), const QString &_message = QString()) : Command("chat_say"), channel(_channel), message(_message)
	{
		parameters.insert("channel", channel);
		parameters.insert("message", message);
	}
};



#endif
