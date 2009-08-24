#ifndef CHATCHANNEL_H
#define CHATCHANNEL_H

#include <QList>
#include <QObject>
#include <QStringList>

class ServerSocket;

class ChatChannel : public QObject, public QList<ServerSocket *> {
	Q_OBJECT
signals:
	void channelInfoChanged();
private:
	QString name;
	QString description;
	bool autoJoin;
	QStringList joinMessage;
public:
	ChatChannel(const QString &_name, const QString &_description, bool _autoJoin, const QStringList &_joinMessage);
	QString getName() const { return name; }
	QString getDescription() const { return description; }
	bool getAutoJoin() const { return autoJoin; }
	void addPlayer(ServerSocket *player);
	void removePlayer(ServerSocket *player);
	void say(ServerSocket *player, const QString &s);
	QString getChannelListLine() const;
};

#endif
