#ifndef SERVER_DECK_VALIDATION_STRATEGY_H
#define SERVER_DECK_VALIDATION_STRATEGY_H

#include <libcockatrice/protocol/pb/response.pb.h>

class DeckList;
class Server_Game;
class Server_Player;
class ResponseContainer;

class Server_DeckValidationStrategy
{
public:
    virtual ~Server_DeckValidationStrategy() = default;

    virtual Response::ResponseCode
    validate(Server_Game *game, Server_Player *player, DeckList *deck, ResponseContainer &rc) = 0;
};

class Server_DefaultDeckValidationStrategy : public Server_DeckValidationStrategy
{
public:
    Response::ResponseCode validate(Server_Game *, Server_Player *, DeckList *, ResponseContainer &) override
    {
        return Response::RespOk;
    }
};

#endif
