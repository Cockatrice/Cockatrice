/**
 * @file player.h
 * @ingroup GameLogicPlayers
 */
//! \todo Document this file.

#ifndef PLAYER_H
#define PLAYER_H

#include "../../game_graphics/player/player_area.h"
#include "../../interface/widgets/menus/tearoff_menu.h"
#include "../board/arrow_data.h"
#include "../interface/deck_loader/loaded_deck.h"
#include "../zones/command_zone_logic.h"
#include "../zones/hand_zone_logic.h"
#include "../zones/pile_zone_logic.h"
#include "../zones/stack_zone_logic.h"
#include "../zones/table_zone_logic.h"
#include "player_event_handler.h"
#include "player_info.h"

#include <QInputDialog>
#include <QLoggingCategory>
#include <QMap>
#include <QTimer>
#include <libcockatrice/filters/filter_string.h>
#include <libcockatrice/protocol/pb/card_attributes.pb.h>
#include <libcockatrice/protocol/pb/game_event.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_card.pb.h>
#include <libcockatrice/utility/zone_names.h>

inline Q_LOGGING_CATEGORY(PlayerLog, "player");

namespace google
{
namespace protobuf
{
class Message;
}
} // namespace google
class AbstractCardItem;
class AbstractGame;
class ArrowItem;
class ArrowTarget;
class CardDatabase;
class CardZone;
class CommandContainer;
class GameCommand;
class GameEvent;
class PlayerInfo;
class PlayerEventHandler;
class PlayerActions;
class PlayerMenu;
class QAction;
class QMenu;
class ServerInfo_Arrow;
class ServerInfo_Card;
class ServerInfo_Counter;
class ServerInfo_Player;
class ServerInfo_User;
class TabGame;
class AbstractCounter;

const int MAX_TOKENS_PER_DIALOG = 99;

class PlayerLogic : public QObject
{
    Q_OBJECT

signals:
    void openDeckEditor(const LoadedDeck &deck);
    void requestZoneViewToggle(PlayerLogic *player, const QString &zoneName, int numberCards, bool isReversed);
    void requestRevealedZoneView(PlayerLogic *player,
                                 CardZoneLogic *zone,
                                 const QList<const ServerInfo_Card *> &cardList,
                                 bool withWritePermission);
    void deckChanged();
    void newCardAdded(AbstractCardItem *card);
    void requestCardMenuUpdate(const CardItem *card);
    void counterAdded(CounterState *state);
    void counterRemoved(int counterId);
    void rearrangeCounters();
    void activeChanged(bool active);
    void zoneIdChanged(int zoneId);
    void concededChanged(int playerId, bool conceded);
    void clearCustomZonesMenu();
    void addViewCustomZoneActionToCustomZoneMenu(QString zoneName);
    void resetTopCardMenuActions();
    void arrowCreateRequested(QSharedPointer<ArrowData> data);
    void arrowDeleteRequested(int creatorId, int arrowId);
    void arrowDeleted(int creatorId, int arrowId);
    void arrowsClearedLocally(); // fires on clear() and processPlayerInfo
    void commandZoneSupportChanged(bool hasCommandZone);

public slots:
    void setActive(bool _active);
    void onRequestZoneViewToggle(const QString &zoneName, int numberCards, bool isReversed);

public:
    PlayerLogic(const ServerInfo_User &info, int _id, bool _local, bool _judge, AbstractGame *_parent);
    ~PlayerLogic() override;

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

    [[nodiscard]] PlayerActions *getPlayerActions() const
    {
        return playerActions;
    }

    [[nodiscard]] PlayerEventHandler *getPlayerEventHandler() const
    {
        return playerEventHandler;
    }

    [[nodiscard]] PlayerInfo *getPlayerInfo() const
    {
        return playerInfo;
    }

    void setDeck(const DeckList &_deck);

    [[nodiscard]] const DeckList &getDeck() const
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
        return qobject_cast<PileZoneLogic *>(zones.value(ZoneNames::DECK));
    }

    PileZoneLogic *getGraveZone()
    {
        return qobject_cast<PileZoneLogic *>(zones.value(ZoneNames::GRAVE));
    }

    PileZoneLogic *getRfgZone()
    {
        return qobject_cast<PileZoneLogic *>(zones.value(ZoneNames::EXILE));
    }

    PileZoneLogic *getSideboardZone()
    {
        return qobject_cast<PileZoneLogic *>(zones.value(ZoneNames::SIDEBOARD));
    }

    TableZoneLogic *getTableZone()
    {
        return qobject_cast<TableZoneLogic *>(zones.value(ZoneNames::TABLE));
    }

    StackZoneLogic *getStackZone()
    {
        return qobject_cast<StackZoneLogic *>(zones.value(ZoneNames::STACK));
    }

    HandZoneLogic *getHandZone()
    {
        return qobject_cast<HandZoneLogic *>(zones.value(ZoneNames::HAND));
    }

    /** @brief Returns the command zone logic, or nullptr if not present. */
    CommandZoneLogic *getCommandZone()
    {
        return qobject_cast<CommandZoneLogic *>(zones.value(ZoneNames::COMMAND));
    }

    /** @brief Whether the server confirmed command zone support for this game. */
    bool hasServerCommandZone() const
    {
        return serverHasCommandZone;
    }

    CounterState *addCounter(const ServerInfo_Counter &counter);
    CounterState *
    addCounter(int id, const QString &name, const QColor &color, int radius, int value, bool active = true);
    void delCounter(int counterId);
    void clearCounters();

    QMap<int, CounterState *> getCounters() const
    {
        return counters;
    }

    /**
     * Gets the counter that represents the life total.
     */
    CounterState *getLifeCounter() const;

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

    bool active;
    bool conceded;

    DeckList deck;

    int zoneId;
    QMap<QString, CardZoneLogic *> zones;
    QMap<int, CounterState *> counters;

    bool dialogSemaphore;
    bool serverHasCommandZone;
    QList<CardItem *> cardsToDelete;
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
