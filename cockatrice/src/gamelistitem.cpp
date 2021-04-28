#include "gamelistitem.h"
#include "tab_room.h"
#include "gamesmodel.h"
#include "pb/event_list_games.pb.h"
#include "pb/serverinfo_game.pb.h"
#include "pixmapgenerator.h"

#include <QString>

static constexpr int ICON_HEIGHT = 13;

GameListItem::GameListItem(TabRoom *_room, const ServerInfo_Game &_game, const GameTypeMap &_gameTypeMap) : room(_room), game(), gameTypeMap(_gameTypeMap){
    update(_game);
}

bool GameListItem::update(const ServerInfo_Game &_game) {
    game.MergeFrom(_game);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
    then = QDateTime::fromSecsSinceEpoch(game.start_time(), Qt::UTC);
#else
    then = QDateTime::fromTime_t(game.start_time(), Qt::UTC);
#endif
    full = game.player_count() == game.max_players();
    lockedIcon = game.with_password() ? QIcon(LockPixmapGenerator::generatePixmap(ICON_HEIGHT)) : QVariant();
    players = QString("%1/%2").arg(game.player_count()).arg(game.max_players());

    QStringList result;
    for (int i = game.game_types_size() - 1; i >= 0; --i)
        result.append(gameTypeMap.value(game.game_types(i)));
    types =  result.join(", "); // game types are set by server and not translatable

    QPixmap avatarPixmap = UserLevelPixmapGenerator::generatePixmap(
            ICON_HEIGHT, (UserLevelFlags)game.creator_info().user_level(), false,
            QString::fromStdString(game.creator_info().privlevel()));
    creatorIcon  = QIcon(avatarPixmap);

    result.clear();
    if (game.with_password())
        result.append(GamesModel::tr("password"));
    if (game.only_buddies())
        result.append(GamesModel::tr("buddies only"));
    if (game.only_registered())
        result.append(GamesModel::tr("reg. users only"));
    restrictions = result.join(GamesModel::tr(", ", "used for joining list of game restrictions"));

    if (game.spectators_allowed()) {
        static const QString braces("(%1)");
        spectators = QString::number(game.spectators_count());

        if (game.spectators_can_chat() && game.spectators_omniscient()) {
            spectators.append(braces.arg(GamesModel::tr("can chat & see hands")));
        } else if (game.spectators_can_chat()) {
            spectators.append(braces.arg(GamesModel::tr("can chat")));
        } else if (game.spectators_omniscient()) {
            spectators.append(braces.arg(GamesModel::tr("can see hands")));
        }
    } else {
        spectators = GamesModel::tr("not allowed");
    }
    if (onUpdate) {
    onUpdate();
    }

    return game.closed();
}

void GameListItem::updateFunc(std::function<void()> func) {
    onUpdate = std::move(func);
}

QVariant GameListItem::data(int column, int role) {
    switch (column) {
        case GamesModel::ROOM:
            return room->getRoomId();
        case GamesModel::CREATED: {
                                      switch (role) {
                                          case Qt::DisplayRole: {
                                                                    int secs = then.secsTo(QDateTime::currentDateTimeUtc());
                                                                    return GamesModel::getGameCreatedString(secs);
                                                                }
                                          case GamesModel::SORT_ROLE:
                                                                return QVariant(-static_cast<qint64>(game.start_time()));
                                          case Qt::TextAlignmentRole:
                                                                return Qt::AlignCenter;
                                          default:
                                                                return QVariant();
                                      }
                                  }
        case GamesModel::DESCRIPTION:
                                  switch (role) {
                                      case GamesModel::SORT_ROLE:
                                      case Qt::DisplayRole:
                                          return QString::fromStdString(game.description());
                                      case Qt::TextAlignmentRole:
                                          return Qt::AlignLeft;
                                      default:
                                          return QVariant();
                                  }
        case GamesModel::CREATOR: {
                                      switch (role) {
                                          case GamesModel::SORT_ROLE:
                                          case Qt::DisplayRole:
                                              return QString::fromStdString(game.creator_info().name());
                                          case Qt::DecorationRole: 
                                              return creatorIcon;
                                          default:
                                              return QVariant();
                                      }
                                  }
        case GamesModel::GAME_TYPE:
                                  switch (role) {
                                      case GamesModel::SORT_ROLE:
                                      case Qt::DisplayRole: 
                                          return types;
                                      case Qt::TextAlignmentRole:
                                          return Qt::AlignLeft;
                                      default:
                                          return QVariant();
                                  }
        case GamesModel::RESTRICTIONS:
                                  switch (role) {
                                      case GamesModel::SORT_ROLE:
                                      case Qt::DisplayRole: 
                                          return restrictions;
                                      case Qt::DecorationRole: 
                                          return lockedIcon;
                                      case Qt::TextAlignmentRole:
                                          return Qt::AlignLeft;
                                      default:
                                          return QVariant();
                                  }
        case GamesModel::PLAYERS:
                                  switch (role) {
                                      case GamesModel::SORT_ROLE:
                                      case Qt::DisplayRole:
                                          return players;
                                      case Qt::TextAlignmentRole:
                                          return Qt::AlignCenter;
                                      default:
                                          return QVariant();
                                  }

        case GamesModel::SPECTATORS:
                                  switch (role) {
                                      case GamesModel::SORT_ROLE:
                                      case Qt::DisplayRole: 
                                          return spectators;
                                      case Qt::TextAlignmentRole:
                                          return Qt::AlignLeft;
                                      default:
                                          return QVariant();
                                  }
        default:
                                  return QVariant();
    }
}
