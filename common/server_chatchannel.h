#ifndef CHATCHANNEL_H
#define CHATCHANNEL_H

#include <QList>
#include <QObject>
#include <QStringList>

class Server_ProtocolHandler;

class Server_ChatChannel : public QObject, public QList<Server_ProtocolHandler *> {
	Q_OBJECT
signals:
	void channelInfoChanged();
private:
	QString name;
	QString description;
	bool autoJoin;
	QStringList joinMessage;
public:
	Server_ChatChannel(const QString &_name, const QString &_description, bool _autoJoin, const QStringList &_joinMessage);
	QString getName() const { return name; }
	QString getDescription() const { return description; }
	bool getAutoJoin() const { return autoJoin; }
	void addClient(Server_ProtocolHandler *client);
	void removeClient(Server_ProtocolHandler *client);
	void say(Server_ProtocolHandler *client, const QString &s);
	QString getChannelListLine() const;
};

#endif
