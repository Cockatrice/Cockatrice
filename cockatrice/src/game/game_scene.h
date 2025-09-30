#ifndef GAMESCENE_H
#define GAMESCENE_H

#include "zones/logic/card_zone_logic.h"

#include <QGraphicsScene>
#include <QList>
#include <QLoggingCategory>
#include <QPointer>
#include <QSet>

inline Q_LOGGING_CATEGORY(GameSceneLog, "game_scene");
inline Q_LOGGING_CATEGORY(GameScenePlayerAdditionRemovalLog, "game_scene.player_addition_removal");

class Player;
class PlayerGraphicsItem;
class ZoneViewWidget;
class CardZone;
class AbstractCardItem;
class CardItem;
class ServerInfo_Card;
class PhasesToolbar;
class QBasicTimer;

/**
 * @class GameScene
 * @ingroup GameGraphics
 * @brief Manages the game board display including players, zones, cards, and animations.
 *
 * GameScene handles:
 * - Dynamic arrangement of players in columns/rows.
 * - Player rotation adjustments.
 * - Zone views for cards (e.g., graveyard, library, revealed zones).
 * - Hover and animation handling for cards.
 * - Scene resizing and responsive layout.
 */
class GameScene : public QGraphicsScene
{
    Q_OBJECT
private:
    static const int playerAreaSpacing = 5; ///< Space between player areas

    PhasesToolbar *phasesToolbar;                       ///< Toolbar showing game phases
    QList<PlayerGraphicsItem *> players;                ///< All player graphics items
    QList<QList<PlayerGraphicsItem *>> playersByColumn; ///< Players organized by column
    QList<ZoneViewWidget *> zoneViews;                  ///< Active zone view widgets
    QSize viewSize;                                     ///< Current view size
    QPointer<CardItem> hoveredCard;                     ///< Currently hovered card
    QBasicTimer *animationTimer;                        ///< Timer for card animations
    QSet<CardItem *> cardsToAnimate;                    ///< Cards currently animating
    int playerRotation;                                 ///< Rotation offset for player layout

    /**
     * @brief Updates which card is currently hovered based on scene coordinates.
     * @param scenePos Scene position of the cursor.
     */
    void updateHover(const QPointF &scenePos);

public:
    /**
     * @brief Constructs the GameScene.
     * @param _phasesToolbar Toolbar widget for phases.
     * @param parent Optional parent QObject.
     */
    explicit GameScene(PhasesToolbar *_phasesToolbar, QObject *parent = nullptr);

    /** Destructor, cleans up timer and zone views. */
    ~GameScene() override;

    /** Updates UI text for all zone views. */
    void retranslateUi();

    /**
     * @brief Adds a player to the scene and stores their graphics item.
     * @param player Player to add.
     */
    void addPlayer(Player *player);

    /**
     * @brief Removes a player from the scene.
     * @param player Player to remove.
     */
    void removePlayer(Player *player);

    /**
     * @brief Adjusts the global rotation offset for player layout.
     * @param rotationAdjustment Number of positions to rotate.
     */
    void adjustPlayerRotation(int rotationAdjustment);

    /** Recomputes the layout of players and the scene size. */
    void rearrange();

    /**
     * @brief Handles view resize and redistributes player positions.
     * @param newSize New view size.
     */
    void processViewSizeChange(const QSize &newSize);

    /**
     * @brief Collects all active (non-conceded) players.
     * @param firstPlayerIndex Output index of first local player.
     * @return List of active players.
     */
    QList<Player *> collectActivePlayers(int &firstPlayerIndex) const;

    /**
     * @brief Rotates the list of players for layout.
     * @param players Original list of players.
     * @param firstPlayerIndex Index of first local player.
     * @return Rotated list.
     */
    QList<Player *> rotatePlayers(const QList<Player *> &players, int firstPlayerIndex) const;

    /**
     * @brief Determines the number of columns to display players in.
     * @param playerCount Total number of active players.
     * @return Number of columns (1 or 2).
     */
    static int determineColumnCount(int playerCount);

    /**
     * @brief Computes layout positions and scene size based on players and columns.
     * @param playersPlaying List of active players.
     * @param columns Number of columns to split into.
     * @return Calculated scene size.
     */
    QSizeF computeSceneSizeAndPlayerLayout(const QList<Player *> &playersPlaying, int columns);

    /**
     * @brief Computes the minimum width for each column based on player minimum widths.
     * @return List of minimum widths per column.
     */
    QList<qreal> calculateMinWidthByColumn() const;

    /**
     * @brief Calculates new scene width considering window aspect ratio.
     * @param newSize View size.
     * @param minWidth Minimum width needed to fit all players.
     * @return Scene width respecting window and content.
     */
    qreal calculateNewSceneWidth(const QSize &newSize, qreal minWidth) const;

    /**
     * @brief Resizes columns and distributes extra width to players.
     * @param minWidthByColumn Minimum widths per column.
     * @param newWidth Total scene width.
     */
    void resizeColumnsAndPlayers(const QList<qreal> &minWidthByColumn, qreal newWidth);

    /** Finds the topmost card zone under the cursor. */
    static CardZone *findTopmostZone(const QList<QGraphicsItem *> &items);

    /** Finds the topmost card in a given zone, considering attachments and Z-order. */
    static CardItem *findTopmostCardInZone(const QList<QGraphicsItem *> &items, CardZone *zone);

    /** Updates hovered card highlighting. */
    void updateHoveredCard(CardItem *newCard);

    /** Registers a card for animation updates. */
    void registerAnimationItem(AbstractCardItem *card);

    /** Unregisters a card from animation updates. */
    void unregisterAnimationItem(AbstractCardItem *card);
    void startRubberBand(const QPointF &selectionOrigin);
    void resizeRubberBand(const QPointF &cursorPoint);
    void stopRubberBand();

public slots:
    /** Toggles a zone view for a player. */
    void toggleZoneView(Player *player, const QString &zoneName, int numberCards, bool isReversed = false);

    /** Adds a revealed zone view (for shown cards). */
    void addRevealedZoneView(Player *player,
                             CardZoneLogic *zone,
                             const QList<const ServerInfo_Card *> &cardList,
                             bool withWritePermission);

    /** Removes a zone view widget from the scene. */
    void removeZoneView(ZoneViewWidget *item);

    /** Closes all zone views. */
    void clearViews();

    /** Closes the most recently added zone view. */
    void closeMostRecentZoneView();
    QTransform getViewTransform() const;
    QTransform getViewportTransform() const;

protected:
    /** Handles hover updates. */
    bool event(QEvent *event) override;

    /** Handles animation timer updates. */
    void timerEvent(QTimerEvent *event) override;

signals:
    void sigStartRubberBand(const QPointF &selectionOrigin);
    void sigResizeRubberBand(const QPointF &cursorPoint);
    void sigStopRubberBand();
};

#endif
