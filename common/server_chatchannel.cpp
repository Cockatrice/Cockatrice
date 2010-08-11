#include "server_chatchannel.h"
#include "server_protocolhandler.h"

Server_ChatChannel::Server_ChatChannel(const QString &_name, const QString &_description, bool _autoJoin, const QString &_joinMessage)
	: name(_name), description(_description), autoJoin(_autoJoin), joinMessage(_joinMessage)
{
}

void Server_ChatChannel::addClient(Server_ProtocolHandler *client)
{
	sendChatEvent(new Event_ChatJoinChannel(name, client->getPlayerName()));
	append(client);
	
	QList<ServerInfo_ChatUser *> eventUserList;
	for (int i = 0; i < size(); ++i)
		eventUserList.append(new ServerInfo_ChatUser(at(i)->getPlayerName()));
	Event_ChatListPlayers *eventCLP = new Event_ChatListPlayers(name, eventUserList);
	client->enqueueProtocolItem(eventCLP);
	
	client->enqueueProtocolItem(new Event_ChatSay(name, QString(), joinMessage));

	emit channelInfoChanged();
}

void Server_ChatChannel::removeClient(Server_ProtocolHandler *client)
{
	removeAt(indexOf(client));
	sendChatEvent(new Event_ChatLeaveChannel(name, client->getPlayerName()));
	emit channelInfoChanged();
}

void Server_ChatChannel::say(Server_ProtocolHandler *client, const QString &s)
{
	sendChatEvent(new Event_ChatSay(name, client->getPlayerName(), s));
}

void Server_ChatChannel::sendChatEvent(ChatEvent *event)
{
	for (int i = 0; i < size(); ++i)
		at(i)->sendProtocolItem(event, false);
	delete event;
}
