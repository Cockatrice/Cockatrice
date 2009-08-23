#include "chatchannel.h"
#include "serversocket.h"

ChatChannel::ChatChannel(const QString &_name, const QString &_description)
	: name(_name), description(_description)
{
}

void ChatChannel::addPlayer(ServerSocket *player)
{
	QString str = QString("chat|join_channel|%1|%2").arg(name).arg(player->getPlayerName());
	for (int i = 0; i < size(); ++i)
		at(i)->msg(str);
	
	append(player);
	
	for (int i = 0; i < size(); ++i)
		player->msg(QString("chat|list_players|%1|%2").arg(name).arg(at(i)->getPlayerName()));

	emit channelInfoChanged();
}

void ChatChannel::removePlayer(ServerSocket *player)
{
	QString str = QString("chat|leave_channel|%1|%2").arg(name).arg(player->getPlayerName());
	
	removeAt(indexOf(player));
	for (int i = 0; i < size(); ++i)
		at(i)->msg(str);
	
	emit channelInfoChanged();
}

void ChatChannel::say(ServerSocket *player, const QString &s)
{
	QString str = QString("chat|say|%1|%2|%3").arg(name).arg(player->getPlayerName()).arg(s);
	for (int i = 0; i < size(); ++i)
		at(i)->msg(str);
}

QString ChatChannel::getChannelListLine() const
{
	return QString("chat|list_channels|%1|%2|%3").arg(name).arg(description).arg(size());
}