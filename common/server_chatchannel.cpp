#include "server_chatchannel.h"
#include "server_protocolhandler.h"

Server_ChatChannel::Server_ChatChannel(const QString &_name, const QString &_description, bool _autoJoin, const QString &_joinMessage)
	: name(_name), description(_description), autoJoin(_autoJoin), joinMessage(_joinMessage)
{
}

void Server_ChatChannel::addClient(Server_ProtocolHandler *client)
{
	sendChatEvent(new Event_ChatJoinChannel(name, new ServerInfo_User(client->getUserInfo())));
	append(client);
	
	QList<ServerInfo_User *> eventUserList;
	for (int i = 0; i < size(); ++i)
		eventUserList.append(new ServerInfo_User(at(i)->getUserInfo()));
	Event_ChatListPlayers *eventCLP = new Event_ChatListPlayers(name, eventUserList);
	client->enqueueProtocolItem(eventCLP);
	
	client->enqueueProtocolItem(new Event_ChatSay(name, QString(), joinMessage));

	emit channelInfoChanged();
}

void Server_ChatChannel::removeClient(Server_ProtocolHandler *client)
{
	removeAt(indexOf(client));
	sendChatEvent(new Event_ChatLeaveChannel(name, client->getUserInfo()->getName()));
	emit channelInfoChanged();
}

void Server_ChatChannel::say(Server_ProtocolHandler *client, const QString &s)
{
	sendChatEvent(new Event_ChatSay(name, client->getUserInfo()->getName(), s));
}

void Server_ChatChannel::sendChatEvent(ChatEvent *event)
{
	for (int i = 0; i < size(); ++i)
		at(i)->sendProtocolItem(event, false);
	delete event;
}
