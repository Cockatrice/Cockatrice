/**
 * @file player.h
 * @ingroup GameLogicPlayers
 */
//! \todo Document this file.

#ifndef PLAYER_H
#define PLAYER_H

#include "../zones/card_zone_logic.h"
#include "../zones/hand_zone_logic.h"
#include "../zones/pile_zone_logic.h"
#include "../zones/stack_zone_logic.h"
#include "../zones/table_zone_logic.h"
#include "libcockatrice/deck_list/deck_list.h"
#include "libcockatrice/utility/card_ref.h"
#include "libcockatrice/utility/playmat_params.h"

#include <QColor>
#include <QInputDialog>
#include <QList>
#include <QLoggingCategory>
#include <QMap>
#include <QObject>
#include <QString>
#include <libcockatrice/utility/zone_names.h>
#include <qtmetamacros.h>

class CardItem;
class CounterState;
class QWidget;
class ServerInfo_PlayerProperties;
struct ArrowData;
struct LoadedDeck;
template <class T> class QSharedPointer;

inline Q_LOGGING_CATEGORY(PlayerLog, "player");
class AbstractCardItem;
class AbstractGame;
class PlayerInfo;
class PlayerEventHandler;
class PlayerActions;
class ServerInfo_Card;
class ServerInfo_Counter;
class ServerInfo_Player;
class ServerInfo_User;

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
    /** @brief Emitted when the remote playmat (card/params) is updated from player properties. */
    void playmatChanged();
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

    CounterState *addCounter(const ServerInfo_Counter &counter);
    CounterState *addCounter(int id, const QString &name, const QColor &color, int radius, int value);
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

    void setPlaymatFromProperties(const ServerInfo_PlayerProperties &props);
    const CardRef &getRemotePlaymatCard() const
    {
        return remotePlaymatCard;
    }
    const PlaymatParams &getRemotePlaymatParams() const
    {
        return remotePlaymatParams;
    }
    bool getHasRemotePlaymat() const
    {
        return hasRemotePlaymat;
    }

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
    QList<CardItem *> cardsToDelete;

    // Playmat from player properties (for opponent display)
    CardRef remotePlaymatCard;
    PlaymatParams remotePlaymatParams;
    bool hasRemotePlaymat = false;
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
