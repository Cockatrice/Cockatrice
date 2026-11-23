/** @file servatrice_in_memory_database_test.cpp
 *  @brief Tests for the in-memory server database shared between the main interface and its workers.
 *  @ingroup Tests
 */

#include "libcockatrice/server/dev/servatrice_in_memory_database_interface.h"

#include <QString>
#include <gtest/gtest.h>
#include <libcockatrice/rng/rng_abstract.h>
#include <memory>

RNG_Abstract *rng = nullptr; // referenced by the server_remote library

namespace
{

TEST(ServatriceInMemoryDatabaseTest, WorkersShareTheSeededState)
{
    Servatrice_InMemoryDatabaseInterface mainDatabase;
    mainDatabase.setAuthenticationResult(UnknownUser);

    ServerInfo_User user;
    user.set_name("tester");
    user.set_id(7);
    mainDatabase.addUser(user);

    Server_DatabaseInterface::BootstrapRoomRecord room;
    room.id = 3;
    room.name = "Lobby";
    mainDatabase.addRoom(room);

    std::unique_ptr<Server_DatabaseInterface> worker(mainDatabase.createWorkerInterface(0));
    ASSERT_NE(nullptr, worker);

    QString reason;
    int secondsLeft = 0;
    EXPECT_EQ(UnknownUser, worker->checkUserPassword(nullptr, "tester", "", "", reason, secondsLeft, false));

    const ServerInfo_User fetched = worker->getUserData("tester", true);
    EXPECT_EQ(7, fetched.id());
    EXPECT_FALSE(worker->getUserData("tester", false).has_id());

    const auto rooms = worker->getBootstrapRooms(0);
    ASSERT_EQ(1, rooms.size());
    EXPECT_EQ(3, rooms.first().id);
    EXPECT_EQ("Lobby", rooms.first().name);

    // A worker records the messages it logs, and the main interface sees them.
    worker->logMessage(1, "tester", "127.0.0.1", "hello", Server_DatabaseInterface::MessageTargetRoom, 3, "Lobby");
    EXPECT_EQ(1, mainDatabase.getLogEntries().size());
}

TEST(ServatriceInMemoryDatabaseTest, UnknownUsersAreNamedLikeTheMySqlImplementation)
{
    Servatrice_InMemoryDatabaseInterface database;

    const ServerInfo_User guest = database.getUserData("stranger", true);
    EXPECT_EQ("stranger", QString::fromStdString(guest.name()));
    EXPECT_EQ(ServerInfo_User::IsUser, guest.user_level());
}

TEST(ServatriceInMemoryDatabaseTest, IdsAdvancePerInterfaceState)
{
    Servatrice_InMemoryDatabaseInterface database;

    EXPECT_EQ(1, database.getNextGameId());
    EXPECT_EQ(2, database.getNextGameId());
    EXPECT_EQ(1, database.getNextReplayId());
}

} // namespace
