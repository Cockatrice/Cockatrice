/**
 * @file player.h
 * @ingroup GameLogicPlayers
 * @brief TODO: Document this.
 */

#ifndef PLAYER_H
#define PLAYER_H

#include "../../filters/filter_string.h"
#include "../../interface/widgets/menus/tearoff_menu.h"
#include "../board/abstract_graphics_item.h"
#include "../dialogs/dlg_create_token.h"
#include "menu/player_menu.h"
#include "player_actions.h"
#include "player_area.h"
#include "player_event_handler.h"
#include "player_graphics_item.h"
#include "player_info.h"

#include <QInputDialog>
#include <QLoggingCategory>
#include <QMap>
#include <QPoint>
#include <QTimer>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/protocol/pb/card_attributes.pb.h>
#include <libcockatrice/protocol/pb/game_event.pb.h>

inline Q_LOGGING_CATEGORY(PlayerLog, "player");

namespace google
{
namespace protobuf
{
class Message;
}
} // namespace google
class AbstractCardItem;
class AbstractCounter;
class AbstractGame;
class ArrowItem;
class ArrowTarget;
class CardDatabase;
class CardZone;
class CommandContainer;
class DeckLoader;
class GameCommand;
class GameEvent;
class PlayerInfo;
class PlayerEventHandler;
class PlayerActions;
class PlayerMenu;
class QAction;
class QMenu;
class ServerInfo_Arrow;
class ServerInfo_Counter;
class ServerInfo_Player;
class ServerInfo_User;
class TabGame;

const int MAX_TOKENS_PER_DIALOG = 99;

class Player : public QObject
{
    Q_OBJECT

signals:
    void openDeckEditor(const DeckLoader *deck);
    void deckChanged();
    void newCardAdded(AbstractCardItem *card);
    void rearrangeCounters();
    void activeChanged(bool active);
    void concededChanged(int playerId, bool conceded);
    void clearCustomZonesMenu();
    void addViewCustomZoneActionToCustomZoneMenu(QString zoneName);
    void resetTopCardMenuActions();

public slots:
    void setActive(bool _active);

public:
    Player(const ServerInfo_User &info, int _id, bool _local, bool _judge, AbstractGame *_parent);
    void forwardActionSignalsToEventHandler();
    ~Player() override;

    void initializeZones();
    void updateZones();
    void clear();

    void processPlayerInfo(const ServerInfo_Player &info);
    void processCardAttachment(const ServerInfo_Player &info);

    void addCard(CardItem *c);
    void deleteCard(CardItem *c);

    bool clearCardsToDelete();

    bool getActive() const
    {
        return active;
    }

    AbstractGame *getGame() const
    {
        return game;
    }

    GameScene *getGameScene();

    [[nodiscard]] PlayerGraphicsItem *getGraphicsItem();

    [[nodiscard]] PlayerActions *getPlayerActions() const
    {
        return playerActions;
    };

    [[nodiscard]] PlayerEventHandler *getPlayerEventHandler() const
    {
        return playerEventHandler;
    }

    [[nodiscard]] PlayerInfo *getPlayerInfo() const
    {
        return playerInfo;
    };

    [[nodiscard]] PlayerMenu *getPlayerMenu() const
    {
        return playerMenu;
    }

    void setDeck(const DeckLoader &_deck);

    [[nodiscard]] DeckLoader *getDeck() const
    {
        return deck;
    }

    template <typename T> T *addZone(T *zone)
    {
        zones.insert(zone->getName(), zone);
        return zone;
    }

    CardZoneLogic *getZone(const QString zoneName)
    {
        return zones.value(zoneName);
    }

    const QMap<QString, CardZoneLogic *> &getZones() const
    {
        return zones;
    }

    PileZoneLogic *getDeckZone()
    {
        return qobject_cast<PileZoneLogic *>(zones.value("deck"));
    }

    PileZoneLogic *getGraveZone()
    {
        return qobject_cast<PileZoneLogic *>(zones.value("grave"));
    }

    PileZoneLogic *getRfgZone()
    {
        return qobject_cast<PileZoneLogic *>(zones.value("rfg"));
    }

    PileZoneLogic *getSideboardZone()
    {
        return qobject_cast<PileZoneLogic *>(zones.value("sb"));
    }

    TableZoneLogic *getTableZone()
    {
        return qobject_cast<TableZoneLogic *>(zones.value("table"));
    }

    StackZoneLogic *getStackZone()
    {
        return qobject_cast<StackZoneLogic *>(zones.value("stack"));
    }

    HandZoneLogic *getHandZone()
    {
        return qobject_cast<HandZoneLogic *>(zones.value("hand"));
    }

    AbstractCounter *addCounter(const ServerInfo_Counter &counter);
    AbstractCounter *addCounter(int counterId, const QString &name, QColor color, int radius, int value);
    void delCounter(int counterId);
    void clearCounters();
    void incrementAllCardCounters();

    QMap<int, AbstractCounter *> getCounters()
    {
        return counters;
    }

    ArrowItem *addArrow(const ServerInfo_Arrow &arrow);
    ArrowItem *addArrow(int arrowId, CardItem *startCard, ArrowTarget *targetItem, const QColor &color);
    void delArrow(int arrowId);
    void removeArrow(ArrowItem *arrow);
    void clearArrows();

    const QMap<int, ArrowItem *> &getArrows() const
    {
        return arrows;
    }

    void setConceded(bool _conceded);
    bool getConceded() const
    {
        return conceded;
    }

    void setGameStarted();

    void setDialogSemaphore(const bool _active)
    {
        dialogSemaphore = _active;
    }

    int getZoneId() const
    {
        return zoneId;
    }

    void setZoneId(int _zoneId);

private:
    AbstractGame *game;
    PlayerInfo *playerInfo;
    PlayerEventHandler *playerEventHandler;
    PlayerActions *playerActions;
    PlayerMenu *playerMenu;
    PlayerGraphicsItem *graphicsItem;

    bool active;
    bool conceded;

    DeckLoader *deck;

    int zoneId;
    QMap<QString, CardZoneLogic *> zones;
    QMap<int, AbstractCounter *> counters;
    QMap<int, ArrowItem *> arrows;

    bool dialogSemaphore;
    QList<CardItem *> cardsToDelete;

    // void eventConnectionStateChanged(const Event_ConnectionStateChanged &event);
};

class AnnotationDialog : public QInputDialog
{
    Q_OBJECT
    void keyPressEvent(QKeyEvent *e) override;

public:
    explicit AnnotationDialog(QWidget *parent = nullptr) : QInputDialog(parent)
    {
    }
};

#endif
