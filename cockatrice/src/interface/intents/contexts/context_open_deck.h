#ifndef COCKATRICE_CONTEXT_OPEN_DECK_H
#define COCKATRICE_CONTEXT_OPEN_DECK_H

#include "context_connect_to_server.h"

#include <QString>

struct ContextOpenDeck
{
    ContextConnectToServer serverContext;
    QString shareToken;
};

#endif // COCKATRICE_CONTEXT_OPEN_DECK_H
