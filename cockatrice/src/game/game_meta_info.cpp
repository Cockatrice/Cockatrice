#include "game_meta_info.h"

#include "game.h"

GameMetaInfo::GameMetaInfo(Game *_game) : QObject(_game), game(_game)
{
}
