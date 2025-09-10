#include "game_meta_info.h"

#include "abstract_game.h"

GameMetaInfo::GameMetaInfo(AbstractGame *_game) : QObject(_game), game(_game)
{
}
