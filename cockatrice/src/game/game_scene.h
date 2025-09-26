/**
 * @file game_scene.h
 * @brief Represents the main game scene, managing players, zones, animations, and layout.
 *
 * The GameScene handles the graphical arrangement of players, cards, and zones.
 * It calculates positions based on the number of players, columns, and rotations.
 */

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
    explicit GameScene(PhasesToolbar *_phasesToolbar, QObject *parent = nullptr);
    ~GameScene() override;

    /** Updates UI text for all zone views. */
    void retranslateUi();

    /** Processes changes in the view size and adjusts layout accordingly. */
    void processViewSizeChange(const QSize &newSize);

    /** Returns the current transform of the main view. */
    QTransform getViewTransform() const;

    /** Returns the viewport transform of the main view. */
    QTransform getViewportTransform() const;

    /** Starts a selection rubber band. */
    void startRubberBand(const QPointF &selectionOrigin);

    /** Resizes the rubber band to follow the cursor. */
    void resizeRubberBand(const QPointF &cursorPoint);

    /** Stops the rubber band selection. */
    void stopRubberBand();

    /**
     * @brief Collects active players who have not conceded.
     * @param firstPlayerIndex Output parameter indicating the first local player index.
     * @return List of active players.
     *
     * This is used to determine player order and layout.
     */
    QList<Player *> collectActivePlayers(int &firstPlayerIndex) const;

    /**
     * @brief Rotates the list of players based on first player and rotation offset.
     * @param players Original list of players.
     * @param firstPlayerIndex Index of the first local player.
     * @return Rotated list of players.
     */
    QList<Player *> rotatePlayers(const QList<Player *> &players, int firstPlayerIndex) const;

    /**
     * @brief Determines the number of columns to display players in.
     * @param playerCount Total number of active players.
     * @return Number of columns (1 or 2).
     */
    static int determineColumnCount(int playerCount);

    /**
     * @brief Computes the total scene size and arranges players.
     * @param playersPlaying List of active players.
     * @param columns Number of columns to arrange players into.
     * @return Calculated scene size.
     *
     * This function:
     * - Splits players into columns and rows.
     * - Calculates maximum column width and scene height.
     * - Positions players within columns with spacing.
     * - Mirrors player graphics when appropriate.
     */
    QSizeF computeSceneSizeAndPlayerLayout(const QList<Player *> &playersPlaying, int columns);

    /** Calculates minimum width required for each column based on player minimum widths. */
    QList<qreal> calculateMinWidthByColumn() const;

    /**
     * @brief Calculates the new scene width based on window size and minimum required width.
     * @param newSize New view size.
     * @param minWidth Minimum scene width needed to fit all players.
     * @return New scene width after layout calculations.
     */
    qreal calculateNewSceneWidth(const QSize &newSize, qreal minWidth) const;

    /**
     * @brief Resizes columns and positions players horizontally within the scene.
     * @param minWidthByColumn Minimum width of each column.
     * @param newWidth Total scene width to distribute.
     *
     * Extra width is evenly distributed across columns to fill available space.
     */
    void resizeColumnsAndPlayers(const QList<qreal> &minWidthByColumn, qreal newWidth);

    /** Finds the topmost card zone under the cursor. */
    static CardZone *findTopmostZone(const QList<QGraphicsItem *> &items);

    /** Finds the topmost card in a given zone, considering attachments and Z-order. */
    static CardItem *findTopmostCardInZone(const QList<QGraphicsItem *> &items, CardZone *zone);

    /** Updates hovered card highlighting. */
    void updateHoveredCard(CardItem *newCard);

    /** Registers a card for animation updates. */
    void registerAnimationItem(AbstractCardItem *item);

    /** Unregisters a card from animation updates. */
    void unregisterAnimationItem(AbstractCardItem *card);

public slots:
    /** Toggles the display of a player's zone view. */
    void toggleZoneView(Player *player, const QString &zoneName, int numberCards, bool isReversed = false);

    /** Adds a zone view showing revealed cards. */
    void addRevealedZoneView(Player *player,
                             CardZoneLogic *zone,
                             const QList<const ServerInfo_Card *> &cardList,
                             bool withWritePermission);

    /** Removes a zone view widget from the scene. */
    void removeZoneView(ZoneViewWidget *item);

    /** Adds a player to the scene and connects signals for layout updates. */
    void addPlayer(Player *player);

    /** Removes a player from the scene and updates layout. */
    void removePlayer(Player *player);

    /** Closes all active zone views. */
    void clearViews();

    /** Closes the most recently opened zone view. */
    void closeMostRecentZoneView();

    /** Adjusts player layout rotation and recomputes positions. */
    void adjustPlayerRotation(int rotationAdjustment);

    /** Recomputes the scene layout. */
    void rearrange();

protected:
    bool event(QEvent *event) override; ///< Handles hover updates.
    void timerEvent(QTimerEvent *event) override; ///< Handles animations.

signals:
    void sigStartRubberBand(const QPointF &selectionOrigin);
    void sigResizeRubberBand(const QPointF &cursorPoint);
    void sigStopRubberBand();
};

#endif
