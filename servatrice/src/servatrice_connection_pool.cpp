#include "servatrice_connection_pool.h"
#include "servatrice_database_interface.h"
#include <QThread>

Servatrice_ConnectionPool::Servatrice_ConnectionPool(Servatrice_DatabaseInterface *_databaseInterface)
	: databaseInterface(_databaseInterface),
	  clientCount(0)
{
}

Servatrice_ConnectionPool::~Servatrice_ConnectionPool()
{
	delete databaseInterface;
	thread()->quit();
}
