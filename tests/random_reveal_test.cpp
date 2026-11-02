#include "game/game_config.h"
#include "game/server_abstract_player.h"
#include "game/server_card.h"
#include "game/server_cardzone.h"
#include "game/server_game.h"
#include "movecard_tests/server_test_helpers.h"
#include "server_abstractuserinterface.h"
#include "server_response_containers.h"
#include "server_room.h"

#include <QCoreApplication>
#include <QSet>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>
#include <gtest/gtest.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/protocol/get_pb_extension.h>
#include <libcockatrice/protocol/pb/command_deck_select.pb.h>
#include <libcockatrice/protocol/pb/command_reveal_cards.pb.h>
#include <libcockatrice/protocol/pb/event_reveal_cards.pb.h>
#include <libcockatrice/protocol/pb/game_event_container.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_zone.pb.h>
#include <libcockatrice/rng/rng_abstract.h>
#include <libcockatrice/utility/zone_names.h>
#include <memory>

RNG_Abstract *rng = nullptr;

namespace
{
class AlternatingRng : public RNG_Abstract
{
    int calls = 0;

public:
    unsigned int rand(int min, int max) override
    {
        EXPECT_LE(min, max);
        return calls++ % 2 == 0 && min < max ? min + 1 : min;
    }
};

class Recipient : public Server_AbstractUserInterface
{
public:
    QList<Event_RevealCards> reveals;
    Recipient(Server *server, const char *name) : Server_AbstractUserInterface(server)
    {
        ServerInfo_User user;
        user.set_name(name);
        setUserInfo(user);
    }
    int getLastCommandTime() const override
    {
        return 0;
    }
    bool addSaidMessageSize(int) override
    {
        return true;
    }
    void sendProtocolItem(const Response &) override
    {
    }
    void sendProtocolItem(const SessionEvent &) override
    {
    }
    void sendProtocolItem(const RoomEvent &) override
    {
    }
    void sendProtocolItem(const GameEventContainer &container) override
    {
        for (const auto &event : container.event_list()) {
            if (event.HasExtension(Event_RevealCards::ext)) {
                reveals.append(event.GetExtension(Event_RevealCards::ext));
            }
        }
    }
};

GameCommand randomReveal(int count, int recipient = -1)
{
    GameCommand command;
    auto *request = command.MutableExtension(Command_RevealRandomCards::ext);
    request->set_zone_name(ZoneNames::HAND);
    request->set_number_of_cards(count);
    if (recipient != -1) {
        request->set_player_id(recipient);
    }
    return command;
}
} // namespace

TEST(RandomReveal, DistinctSamplesPrivacyAndCompatibility)
{
    int argc = 1;
    char name[] = "random-reveal-test";
    char *argv[] = {name, nullptr};
    std::unique_ptr<QCoreApplication> application;
    if (!QCoreApplication::instance()) {
        application = std::make_unique<QCoreApplication>(argc, argv);
    }
    AlternatingRng random;
    rng = &random;
    FakeServer server;
    Recipient owner(&server, "Owner"), target(&server, "Target"), bystander(&server, "Bystander"),
        judge(&server, "Judge"), spectator(&server, "Spectator");
    Server_Room room(0, 0, "", "", "", "", false, "", {}, &server);
    GameConfig config;
    config.creatorInfo = *owner.getUserInfo();
    config.gameId = 1;
    config.maxPlayers = 3;
    config.spectatorsAllowed = true;
    config.spectatorsSeeEverything = false;
    config.shareDecklistsOnLoad = false;
    Server_Game game(config, &room);
    for (auto *recipient : {&owner, &target, &bystander}) {
        ResponseContainer response(1);
        game.addPlayer(recipient, response, false, false, false);
    }
    ResponseContainer judgeResponse(1);
    game.addPlayer(&judge, judgeResponse, true, true, false);
    ResponseContainer spectatorResponse(1);
    game.addPlayer(&spectator, spectatorResponse, true, false, false);
    auto *player = game.getPlayer(0);
    ASSERT_NE(player, nullptr);
    ASSERT_NE(game.getPlayer(1), nullptr);
    ResponseContainer response(2);
    GameEventStorage notStarted;
    EXPECT_EQ(player->processGameCommand(randomReveal(2, 1), response, notStarted), Response::RespGameNotStarted);
    EXPECT_TRUE(notStarted.getGameEventList().isEmpty());
    for (auto *participant : game.getPlayers()) {
        Command_DeckSelect select;
        select.set_deck(DeckList().writeToString_Native().toStdString());
        GameCommand command;
        command.MutableExtension(Command_DeckSelect::ext)->CopyFrom(select);
        GameEventStorage events;
        EXPECT_EQ(participant->processGameCommand(command, response, events), Response::RespOk);
        participant->setReadyStart(true);
    }
    game.startGameIfReady(false);
    QCoreApplication::processEvents();
    ASSERT_TRUE(game.getGameStarted());
    auto *hand = player->getZones().value(ZoneNames::HAND);
    ASSERT_NE(hand, nullptr);
    for (int i = 0; i < 4; ++i) {
        auto *card = new Server_Card({i < 2 ? "Twin" : "Other", QString::number(i)}, player->newCardId(), i, 0);
        hand->insertCard(card, -1, 0);
    }
    const auto original = hand->getCards();
    GameEventStorage privateReveal;
    EXPECT_EQ(player->processGameCommand(randomReveal(2, 1), response, privateReveal), Response::RespOk);
    ASSERT_EQ(privateReveal.getGameEventList().size(), 2);
    EXPECT_EQ(privateReveal.getPrivatePlayerId(), 1);
    privateReveal.sendToGame(&game);
    ASSERT_EQ(owner.reveals.size(), 1);
    ASSERT_EQ(target.reveals.size(), 1);
    ASSERT_EQ(bystander.reveals.size(), 1);
    ASSERT_EQ(judge.reveals.size(), 1);
    ASSERT_EQ(spectator.reveals.size(), 1);
    EXPECT_EQ(target.reveals.last().cards_size(), 2);
    EXPECT_EQ(judge.reveals.last().cards_size(), 2); // existing judge visibility is retained
    EXPECT_NE(target.reveals.last().cards(0).id(), target.reveals.last().cards(1).id());
    EXPECT_EQ(target.reveals.last().cards(0).id(), original.at(1)->getId());
    EXPECT_EQ(target.reveals.last().cards(1).id(), original.at(0)->getId());
    EXPECT_EQ(target.reveals.last().cards(0).name(),
              target.reveals.last().cards(1).name()); // physical copies are distinct
    for (auto *recipient : {&owner, &bystander, &spectator}) {
        EXPECT_EQ(recipient->reveals.last().cards_size(), 0);
        EXPECT_EQ(recipient->reveals.last().number_of_cards(), 2);
        ASSERT_EQ(recipient->reveals.last().card_id_size(), 1);
        EXPECT_EQ(recipient->reveals.last().card_id(0), -2); // no actual card ids leak
        EXPECT_EQ(recipient->reveals.last().other_player_id(), 1);
    }
    EXPECT_EQ(hand->getCards(), original);
    GameEventStorage publicReveal;
    EXPECT_EQ(player->processGameCommand(randomReveal(4), response, publicReveal), Response::RespOk);
    publicReveal.sendToGame(&game);
    for (auto *recipient : {&owner, &target, &bystander, &judge, &spectator}) {
        ASSERT_EQ(recipient->reveals.last().cards_size(), 4);
        QSet<int> ids;
        for (const auto &card : recipient->reveals.last().cards()) {
            ids.insert(card.id());
        }
        EXPECT_EQ(ids.size(), 4);
    }
    EXPECT_EQ(hand->getCards(), original);
    for (int amount : {0, -1, 5, 2147483647}) {
        GameEventStorage invalid;
        EXPECT_NE(player->processGameCommand(randomReveal(amount, 1), response, invalid), Response::RespOk);
        EXPECT_TRUE(invalid.getGameEventList().isEmpty());
    }
    GameEventStorage missingRecipient;
    EXPECT_EQ(player->processGameCommand(randomReveal(2, 999), response, missingRecipient), Response::RespNameNotFound);
    EXPECT_TRUE(missingRecipient.getGameEventList().isEmpty());
    player->setConceded(true);
    GameEventStorage conceded;
    EXPECT_EQ(player->processGameCommand(randomReveal(2, 1), response, conceded), Response::RespContextError);
    EXPECT_TRUE(conceded.getGameEventList().isEmpty());
    player->setConceded(false);
    GameCommand legacy;
    auto *single = legacy.MutableExtension(Command_RevealCards::ext);
    single->set_zone_name(ZoneNames::HAND);
    single->add_card_id(-2);
    single->set_player_id(1);
    GameEventStorage legacyReveal;
    EXPECT_EQ(player->processGameCommand(legacy, response, legacyReveal), Response::RespOk);
    legacyReveal.sendToGame(&game);
    EXPECT_EQ(target.reveals.last().cards_size(), 1);
    EXPECT_EQ(owner.reveals.last().cards_size(), 0);

    // A reader with only the old GameCommand schema keeps the new extension unknown.
    // Existing dispatch returns RespInvalidCommand; it cannot fall back to reveal-all.
    google::protobuf::FileDescriptorProto file;
    GameCommand::descriptor()->file()->CopyTo(&file);
    auto *values = file.mutable_message_type(0)->mutable_enum_type(0)->mutable_value();
    for (int i = values->size() - 1; i >= 0; --i) {
        if (values->Get(i).number() == 1036) {
            values->DeleteSubrange(i, 1);
        }
    }
    google::protobuf::DescriptorPool pool;
    ASSERT_NE(pool.BuildFile(file), nullptr);
    google::protobuf::FileDescriptorProto legacyRevealFile;
    Command_RevealCards::descriptor()->file()->CopyTo(&legacyRevealFile);
    auto *messages = legacyRevealFile.mutable_message_type();
    for (int i = messages->size() - 1; i >= 0; --i) {
        if (messages->Get(i).name() == "Command_RevealRandomCards") {
            messages->DeleteSubrange(i, 1);
        }
    }
    ASSERT_NE(pool.BuildFile(legacyRevealFile), nullptr);
    google::protobuf::DynamicMessageFactory factory(&pool);
    std::unique_ptr<google::protobuf::Message> old(
        factory.GetPrototype(pool.FindMessageTypeByName("GameCommand"))->New());
    ASSERT_TRUE(old->ParseFromString(legacy.SerializeAsString()));
    EXPECT_EQ(getPbExtension(*old), GameCommand::REVEAL_CARDS);
    old->Clear();
    ASSERT_TRUE(old->ParseFromString(randomReveal(2, 1).SerializeAsString()));
    EXPECT_EQ(getPbExtension(*old), -1);
    GameCommand unknown;
    unknown.GetReflection()->MutableUnknownFields(&unknown)->MergeFrom(old->GetReflection()->GetUnknownFields(*old));
    GameEventStorage unsupported;
    EXPECT_EQ(player->processGameCommand(unknown, response, unsupported), Response::RespInvalidCommand);
    EXPECT_TRUE(unsupported.getGameEventList().isEmpty());
    rng = nullptr;
}
