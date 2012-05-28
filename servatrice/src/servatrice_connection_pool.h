#ifndef SERVATRICE_CONNECTION_POOL_H
#define SERVATRICE_CONNECTION_POOL_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>

class Servatrice_DatabaseInterface;

class Servatrice_ConnectionPool : public QObject {
	Q_OBJECT
private:
	Servatrice_DatabaseInterface *databaseInterface;
	bool threaded;
	mutable QMutex clientCountMutex;
	int clientCount;
public:
	Servatrice_ConnectionPool(Servatrice_DatabaseInterface *_databaseInterface);
	~Servatrice_ConnectionPool();
	
	Servatrice_DatabaseInterface *getDatabaseInterface() const { return databaseInterface; }
	
	int getClientCount() const { QMutexLocker locker(&clientCountMutex); return clientCount; }
	void addClient() { QMutexLocker locker(&clientCountMutex); ++clientCount; }
public slots:
	void removeClient() { QMutexLocker locker(&clientCountMutex); --clientCount; }
};

#endif
