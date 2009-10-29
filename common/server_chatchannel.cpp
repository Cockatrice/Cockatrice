#include "server_chatchannel.h"
#include "server_protocolhandler.h"

Server_ChatChannel::Server_ChatChannel(const QString &_name, const QString &_description, bool _autoJoin, const QStringList &_joinMessage)
	: name(_name), description(_description), autoJoin(_autoJoin), joinMessage(_joinMessage)
{
}

void Server_ChatChannel::addClient(Server_ProtocolHandler *client)
{
/*	QString str = QString("chat|join_channel|%1|%2").arg(name).arg(player->getPlayerName());
	for (int i = 0; i < size(); ++i)
		at(i)->msg(str);
	
	append(player);
	
	for (int i = 0; i < size(); ++i)
		player->msg(QString("chat|list_players|%1|%2").arg(name).arg(at(i)->getPlayerName()));
	for (int i = 0; i < joinMessage.size(); ++i)
	  	player->msg(QString("chat|server_message|%1|%2").arg(name).arg(joinMessage[i]));

	emit channelInfoChanged();
*/}

void Server_ChatChannel::removeClient(Server_ProtocolHandler *client)
{
/*	QString str = QString("chat|leave_channel|%1|%2").arg(name).arg(player->getPlayerName());
	
	removeAt(indexOf(player));
	for (int i = 0; i < size(); ++i)
		at(i)->msg(str);
	
	emit channelInfoChanged();
*/}

void Server_ChatChannel::say(Server_ProtocolHandler *client, const QString &s)
{
/*	QString str = QString("chat|say|%1|%2|%3").arg(name).arg(player->getPlayerName()).arg(s);
	for (int i = 0; i < size(); ++i)
		at(i)->msg(str);
*/}

QString Server_ChatChannel::getChannelListLine() const
{
//	return QString("chat|list_channels|%1|%2|%3|%4").arg(name).arg(description).arg(size()).arg(autoJoin ? 1 : 0);
}
