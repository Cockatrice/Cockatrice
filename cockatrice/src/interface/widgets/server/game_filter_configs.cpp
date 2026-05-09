#include "game_filter_configs.h"

bool GameFilterConfigs::isDefault() const
{
    static const GameFilterConfigs DEFAULT = {};
    return *this == DEFAULT;
}