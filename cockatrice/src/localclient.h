#ifndef LOCALCLIENT_H
#define LOCALCLIENT_H

#include "abstractclient.h"

class LocalServerInterface;

class LocalClient : public AbstractClient {
	Q_OBJECT
private:
	LocalServerInterface *lsi;
public:
	LocalClient(LocalServerInterface *_lsi, const QString &_playerName, QObject *parent = 0);
	~LocalClient();
	
	void sendCommandContainer(CommandContainer *cont);
	
private slots:
	void itemFromServer(ProtocolItem *item);
signals:
	void itemToServer(ProtocolItem *item);
};

#endif