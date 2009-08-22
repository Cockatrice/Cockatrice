#ifndef CHATCHANNEL_H
#define CHATCHANNEL_H

#include <QList>
#include <QObject>

class ServerSocket;

class ChatChannel : public QObject, public QList<ServerSocket *> {
	Q_OBJECT
signals:
	void channelInfoChanged();
private:
	QString name;
	QString description;
public:
	ChatChannel(const QString &_name, const QString &_description);
	QString getName() const { return name; }
	QString getDescription() const { return description; }
	void addPlayer(ServerSocket *player);
	void removePlayer(ServerSocket *player);
	void say(ServerSocket *player, const QString &s);
};

#endif
