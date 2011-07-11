#ifndef SERVERSOCKETTHREAD_H
#define SERVERSOCKETTHREAD_H

#include <QThread>

class Servatrice;
class ServerSocketInterface;

class ServerSocketThread : public QThread {
	Q_OBJECT
private:
	Servatrice *server;
	ServerSocketInterface *ssi;
	int socketDescriptor;
public:
	ServerSocketThread(int _socketDescriptor, Servatrice *_server, QObject *parent = 0);
	~ServerSocketThread();
protected:
	void run();
};

#endif
