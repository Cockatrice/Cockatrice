#include "interface/widgets/server/user/user_games_model.h"

#include <QAbstractItemModelTester>
#include <QCoreApplication>
#include <QPersistentModelIndex>
#include <QSignalSpy>
#include <gtest/gtest.h>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/commands.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/response_get_games_of_user.pb.h>
#include <libcockatrice/protocol/pb/server_message.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/rng/rng_abstract.h>
#include <memory>

RNG_Abstract *rng = nullptr; // required by the transitive server library; this suite sends no game commands

namespace
{
class GamesClient : public AbstractClient
{
public:
    QList<CommandContainer> sent;

    GamesClient()
    {
        setStatus(StatusLoggedIn);
    }

    ~GamesClient() override
    {
        qDeleteAll(pendingCommands);
        pendingCommands.clear();
    }

    void changeStatus(ClientStatus status)
    {
        setStatus(status);
    }

    void answer(int request, Response response)
    {
        response.set_cmd_id(sent.at(request).cmd_id());
        ServerMessage message;
        message.set_message_type(ServerMessage::RESPONSE);
        *message.mutable_response() = response;
        processProtocolItem(message);
    }

protected:
    void sendCommandContainer(const CommandContainer &command) override
    {
        sent.append(command);
    }
};

Response gamesResponse(std::initializer_list<int> ids)
{
    Response response;
    response.set_response_code(Response::RespOk);
    auto *games = response.MutableExtension(Response_GetGamesOfUser::ext);
    for (int id : ids) {
        auto *game = games->add_game_list();
        game->set_game_id(id);
        game->set_room_id(7);
    }
    return response;
}

using State = UserGamesModel::State;

TEST(UserGamesModel, OfflineUserDoesNotSendARequest)
{
    GamesClient client;
    UserGamesModel model(&client);
    model.setUser("Alice", false);
    model.refresh();
    EXPECT_TRUE(client.sent.isEmpty());
    EXPECT_EQ(State::Offline, model.getState());
    EXPECT_FALSE(model.canRefresh());
    EXPECT_EQ(0, model.rowCount());
}

TEST(UserGamesModel, PresenceLossInvalidatesTheOutstandingResponse)
{
    GamesClient client;
    UserGamesModel model(&client);
    model.setUser("Alice", true);
    EXPECT_EQ(State::Loading, model.getState());
    EXPECT_FALSE(model.canRefresh());
    model.setUser("Alice", false);
    client.answer(0, gamesResponse({1}));
    EXPECT_EQ(State::Offline, model.getState());
    EXPECT_EQ(0, model.rowCount());

    model.setUser("Alice", true);
    client.answer(1, gamesResponse({2}));
    EXPECT_EQ(State::Ready, model.getState());
    EXPECT_TRUE(model.canRefresh());
    EXPECT_EQ(2, model.data(model.index(0), UserGamesModel::GameRole).value<ServerInfo_Game>().game_id());
}

TEST(UserGamesModel, SwitchingBackToTheSameUserDoesNotAcceptTheirOldRequest)
{
    GamesClient client;
    UserGamesModel model(&client);
    model.setUser("Alice", true);
    model.setUser("Bob", true);
    model.setUser("Alice", true);
    client.answer(2, gamesResponse({3}));
    client.answer(0, gamesResponse({1}));
    client.answer(1, gamesResponse({2}));
    EXPECT_EQ(1, model.rowCount());
    EXPECT_EQ(3, model.data(model.index(0), UserGamesModel::GameRole).value<ServerInfo_Game>().game_id());
}

TEST(UserGamesModel, RefreshSupersedesThePreviousRequest)
{
    GamesClient client;
    UserGamesModel model(&client);
    model.setUser("Alice", true);
    model.refresh();
    client.answer(0, gamesResponse({1}));
    EXPECT_EQ(State::Loading, model.getState());
    EXPECT_EQ(0, model.rowCount());
    client.answer(1, gamesResponse({2}));
    EXPECT_EQ(State::Ready, model.getState());
    EXPECT_EQ(2, model.data(model.index(0), UserGamesModel::GameRole).value<ServerInfo_Game>().game_id());
}

TEST(UserGamesModel, EmptySuccessAndMissingPayloadAreDifferentStates)
{
    GamesClient client;
    UserGamesModel model(&client);
    model.setUser("Alice", true);
    client.answer(0, gamesResponse({}));
    EXPECT_EQ(State::Empty, model.getState());
    EXPECT_TRUE(model.canRefresh());

    model.refresh();
    Response missingPayload;
    missingPayload.set_response_code(Response::RespOk);
    client.answer(1, missingPayload);
    EXPECT_EQ(State::Error, model.getState());
    EXPECT_TRUE(model.canRefresh());
}

TEST(UserGamesModel, FailedRequestsCanBeRetried)
{
    GamesClient client;
    UserGamesModel model(&client);
    model.setUser("Alice", true);
    Response error;
    error.set_response_code(Response::RespNameNotFound);
    client.answer(0, error);
    EXPECT_EQ(State::Error, model.getState());
    model.refresh();
    client.answer(1, gamesResponse({4}));
    EXPECT_EQ(State::Ready, model.getState());
}

TEST(UserGamesModel, DisconnectRejectsLateRepliesAndReconnectFetchesAgain)
{
    GamesClient client;
    UserGamesModel model(&client);
    model.setUser("Alice", true);
    client.changeStatus(StatusDisconnecting);
    client.answer(0, gamesResponse({1}));
    EXPECT_EQ(State::Disconnected, model.getState());
    EXPECT_FALSE(model.canRefresh());
    model.refresh();
    EXPECT_EQ(1, client.sent.size());
    client.changeStatus(StatusLoggedIn);
    client.answer(1, gamesResponse({2}));
    EXPECT_EQ(State::Ready, model.getState());
}

TEST(UserGamesModel, ClientDestructionClearsGamesAndDisablesRefresh)
{
    auto client = std::make_unique<GamesClient>();
    UserGamesModel model(client.get());
    model.setUser("Alice", true);
    client->answer(0, gamesResponse({1}));
    client.reset();
    EXPECT_EQ(State::Disconnected, model.getState());
    EXPECT_EQ(0, model.rowCount());
    EXPECT_FALSE(model.canRefresh());
    model.refresh();
    EXPECT_EQ(State::Disconnected, model.getState());
}

TEST(UserGamesModel, ResetInvalidatesAnOpenGameActionAndPreservesTheModelContract)
{
    GamesClient client;
    UserGamesModel model(&client);
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);
    model.setUser("Alice", true);
    client.answer(0, gamesResponse({1, 2, 3}));
    QPersistentModelIndex selected(model.index(1));
    EXPECT_TRUE(selected.isValid());
    model.refresh();
    EXPECT_FALSE(selected.isValid());
    EXPECT_EQ(0, model.rowCount());
    client.answer(1, gamesResponse({4}));
    EXPECT_EQ(1, model.rowCount());
}

TEST(UserGamesModel, StateListenerCanSelectAnotherUserBeforeTheRequestIsSent)
{
    GamesClient client;
    UserGamesModel model(&client);
    bool switched = false;
    QObject::connect(&model, &UserGamesModel::stateChanged, &model, [&] {
        if (!switched && model.getState() == State::Loading) {
            switched = true;
            model.setUser("Bob", false);
        }
    });
    model.setUser("Alice", true);
    EXPECT_TRUE(client.sent.isEmpty());
    EXPECT_EQ(State::Offline, model.getState());
}

TEST(UserGamesModel, TimedOutResponseCannotReplaceARetry)
{
    GamesClient client;
    UserGamesModel model(&client, 50);
    model.setUser("Alice", true);
    QSignalSpy changes(&model, &UserGamesModel::stateChanged);
    ASSERT_TRUE(changes.wait(1000));
    ASSERT_EQ(State::Error, model.getState());
    EXPECT_TRUE(model.canRefresh());
    model.refresh();
    client.answer(0, gamesResponse({1}));
    EXPECT_EQ(State::Loading, model.getState());
    client.answer(1, gamesResponse({2}));
    EXPECT_EQ(State::Ready, model.getState());
    EXPECT_EQ(2, model.data(model.index(0), UserGamesModel::GameRole).value<ServerInfo_Game>().game_id());
}
} // namespace

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
