#include "libcockatrice/server/dev/servatrice_in_memory_database_interface.h"
#include "server.h"

class FakeServer : public Server
{
public:
    FakeServer()
    {
        setDatabaseInterface(new Servatrice_InMemoryDatabaseInterface());
    }
};
