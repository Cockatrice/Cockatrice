#ifndef SERVER_DECK_VALIDATION_STRATEGY_H
#define SERVER_DECK_VALIDATION_STRATEGY_H

#include <libcockatrice/protocol/pb/response.pb.h>

class DeckList;
class Server_Game;
class Server_Player;
class ResponseContainer;

/**
 * @brief Strategy for validating a player's deck before it is loaded into a game.
 *
 * Subclasses decide whether a deck may be accepted; the default implementation
 * accepts every deck.
 */
class Server_DeckValidationStrategy
{
public:
    virtual ~Server_DeckValidationStrategy() = default;

    /**
     * @brief Validate @p deck for @p player in @p game.
     *
     * @p rc is an out parameter used to attach the response details for a rejected
     * deck (e.g. an error response extension via ResponseContainer::setResponseExtension).
     * @return Response::RespOk when the deck is accepted, an error code otherwise.
     */
    virtual Response::ResponseCode
    validate(Server_Game *game, Server_Player *player, DeckList *deck, ResponseContainer &rc) = 0;
};

/**
 * @brief Default deck validation strategy that accepts every deck.
 */
class Server_DefaultDeckValidationStrategy : public Server_DeckValidationStrategy
{
public:
    Response::ResponseCode validate(Server_Game *, Server_Player *, DeckList *, ResponseContainer &) override
    {
        return Response::RespOk;
    }
};

#endif
