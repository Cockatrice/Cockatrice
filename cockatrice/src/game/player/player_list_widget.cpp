#include "player_list_widget.h"

#include "../../interface/pixel_map_generator.h"
#include "../../interface/widgets/server/user/user_context_menu.h"
#include "../../interface/widgets/server/user/user_list_manager.h"
#include "../../interface/widgets/server/user/user_list_widget.h"
#include "../../interface/widgets/tabs/tab_game.h"
#include "../../interface/widgets/tabs/tab_supervisor.h"

#include <QHeaderView>
#include <QMouseEvent>
#include <libcockatrice/protocol/pb/command_kick_from_game.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_playerproperties.pb.h>
#include <libcockatrice/protocol/pb/session_commands.pb.h>

PlayerListItemDelegate::PlayerListItemDelegate(QObject *const parent) : QStyledItemDelegate(parent)
{
}

bool PlayerListItemDelegate::editorEvent(QEvent *event,
                                         QAbstractItemModel *model,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index)
{
    if ((event->type() == QEvent::MouseButtonPress) && index.isValid()) {
        auto *const mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::RightButton) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            static_cast<PlayerListWidget *>(parent())->showContextMenu(mouseEvent->globalPosition().toPoint(), index);
#else
            static_cast<PlayerListWidget *>(parent())->showContextMenu(mouseEvent->globalPos(), index);
#endif
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

PlayerListTWI::PlayerListTWI() : QTreeWidgetItem(Type)
{
}

bool PlayerListTWI::operator<(const QTreeWidgetItem &other) const
{
    // Sort by spectator/player
    if (data(1, Qt::UserRole) != other.data(1, Qt::UserRole))
        return data(1, Qt::UserRole).toBool();

    // Sort by player ID
    return data(4, Qt::UserRole + 1).toInt() < other.data(4, Qt::UserRole + 1).toInt();
}

PlayerListWidget::PlayerListWidget(TabSupervisor *_tabSupervisor,
                                   AbstractClient *_client,
                                   AbstractGame *_game,
                                   QWidget *parent)
    : QTreeWidget(parent), tabSupervisor(_tabSupervisor), client(_client), game(_game), gameStarted(false)
{
    readyIcon = QPixmap("theme:icons/ready_start");
    notReadyIcon = QPixmap("theme:icons/not_ready_start");
    concededIcon = QPixmap("theme:icons/conceded");
    playerIcon = loadColorAdjustedPixmap("theme:icons/player");
    judgeIcon = loadColorAdjustedPixmap("theme:icons/scales");
    spectatorIcon = loadColorAdjustedPixmap("theme:icons/spectator");
    lockIcon = QPixmap("theme:icons/lock");

    if (tabSupervisor) {
        itemDelegate = new PlayerListItemDelegate(this);
        setItemDelegate(itemDelegate);

        userContextMenu = new UserContextMenu(tabSupervisor, this, game);
        connect(userContextMenu, &UserContextMenu::openMessageDialog, this, &PlayerListWidget::openMessageDialog);
    } else {
        userContextMenu = nullptr;
    }

    setMinimumHeight(40);
    setIconSize(QSize(20, 15));
    setColumnCount(6);
    setColumnWidth(0, 20);
    setColumnWidth(1, 20);
    setColumnWidth(2, 20);
    setColumnWidth(3, 20);
    setColumnWidth(5, 20);
    setHeaderHidden(true);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setItemsExpandable(false);
    retranslateUi();
}

void PlayerListWidget::retranslateUi()
{
}

void PlayerListWidget::addPlayer(const ServerInfo_PlayerProperties &player)
{
    QTreeWidgetItem *newPlayer = new PlayerListTWI;
    players.insert(player.player_id(), newPlayer);
    updatePlayerProperties(player);
    addTopLevelItem(newPlayer);
    sortItems(1, Qt::AscendingOrder);
    resizeColumnToContents(4);
    resizeColumnToContents(5);
}

void PlayerListWidget::updatePlayerProperties(const ServerInfo_PlayerProperties &prop, int playerId)
{
    if (playerId == -1)
        playerId = prop.player_id();

    QTreeWidgetItem *player = players.value(playerId, 0);
    if (!player)
        return;

    bool isSpectator = prop.has_spectator() && prop.spectator();
    if (prop.has_judge() || prop.has_spectator()) {
        if (prop.has_judge() && prop.judge()) {
            player->setIcon(1, judgeIcon);
        } else if (isSpectator) {
            player->setIcon(1, spectatorIcon);
        } else {
            player->setIcon(1, playerIcon);
        }
        player->setData(1, Qt::UserRole, !isSpectator);
    }

    if (!isSpectator) {
        if (prop.has_conceded())
            player->setData(2, Qt::UserRole, prop.conceded());
        if (prop.has_ready_start())
            player->setData(2, Qt::UserRole + 1, prop.ready_start());
        if (prop.has_conceded() || prop.has_ready_start())
            player->setIcon(2, gameStarted ? (prop.conceded() ? concededIcon : QIcon())
                                           : (prop.ready_start() ? readyIcon : notReadyIcon));
    }
    if (prop.has_user_info()) {
        player->setData(3, Qt::UserRole, prop.user_info().user_level());
        player->setIcon(3, UserLevelPixmapGenerator::generateIcon(
                               12, UserLevelFlags(prop.user_info().user_level()), prop.user_info().pawn_colors(), false,
                               QString::fromStdString(prop.user_info().privlevel())));
        player->setText(4, QString::fromStdString(prop.user_info().name()));
        const QString country = QString::fromStdString(prop.user_info().country());
        if (!country.isEmpty())
            player->setIcon(4, QIcon(CountryPixmapGenerator::generatePixmap(12, country)));
        player->setData(4, Qt::UserRole, QString::fromStdString(prop.user_info().name()));
    }

    if (prop.has_player_id())
        player->setData(4, Qt::UserRole + 1, prop.player_id());

    if (!isSpectator) {
        if (prop.has_deck_hash()) {
            player->setText(5, QString::fromStdString(prop.deck_hash()));
        }
        if (prop.has_sideboard_locked())
            player->setIcon(5, prop.sideboard_locked() ? lockIcon : QIcon());
    }
    if (prop.has_ping_seconds())
        player->setIcon(0, QIcon(PingPixmapGenerator::generatePixmap(12, prop.ping_seconds(), 10)));
}

void PlayerListWidget::removePlayer(int playerId)
{
    QTreeWidgetItem *player = players.value(playerId, 0);
    if (!player)
        return;
    players.remove(playerId);
    delete takeTopLevelItem(indexOfTopLevelItem(player));
}

void PlayerListWidget::setActivePlayer(int playerId)
{
    QMapIterator<int, QTreeWidgetItem *> i(players);
    while (i.hasNext()) {
        i.next();
        QTreeWidgetItem *twi = i.value();
        if (i.key() == playerId) {
            twi->setBackground(4, QColor(150, 255, 150));
            twi->setForeground(4, QColor(0, 0, 0));
        } else {
            twi->setBackground(4, palette().base().color());
            twi->setForeground(4, palette().text().color());
        }
    }
}

void PlayerListWidget::setGameStarted(bool _gameStarted, bool resuming)
{
    gameStarted = _gameStarted;
    QMapIterator<int, QTreeWidgetItem *> i(players);
    while (i.hasNext()) {
        QTreeWidgetItem *twi = i.next().value();

        bool isPlayer = twi->data(1, Qt::UserRole).toBool();
        if (!isPlayer)
            continue;

        if (gameStarted) {
            if (resuming)
                twi->setIcon(2, twi->data(2, Qt::UserRole).toBool() ? concededIcon : QIcon());
            else {
                twi->setData(2, Qt::UserRole, false);
                twi->setIcon(2, QIcon());
            }
        } else {
            twi->setIcon(2, notReadyIcon);
        }
    }
}

void PlayerListWidget::showContextMenu(const QPoint &pos, const QModelIndex &index)
{
    if (!userContextMenu)
        return;

    const QString &userName = index.sibling(index.row(), 4).data(Qt::UserRole).toString();
    int playerId = index.sibling(index.row(), 4).data(Qt::UserRole + 1).toInt();
    UserLevelFlags userLevel(index.sibling(index.row(), 3).data(Qt::UserRole).toInt());
    QString deckHash = index.sibling(index.row(), 5).data().toString();

    userContextMenu->showContextMenu(pos, userName, userLevel, true, playerId, deckHash);
}
