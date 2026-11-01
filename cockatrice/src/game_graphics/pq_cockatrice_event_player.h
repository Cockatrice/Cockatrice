#ifndef PQ_COCKATRICE_EVENT_PLAYER_H
#define PQ_COCKATRICE_EVENT_PLAYER_H

#include "pqWidgetEventPlayer.h"

#include <QModelIndex>

class CardItem;
class QAction;
class QGraphicsItem;
class QGraphicsScene;
class QGraphicsView;
class QMenu;

/**
 * Custom event player for Cockatrice game interactions.
 *
 * Card commands are resolution independent: the card is looked up in the
 * QGraphicsScene by name and zone, then mouse events are sent at the card's
 * current viewport position. That keeps recordings usable across screen sizes
 * and window geometries.
 *
 * Commands:
 *   clickCard "cardName,zoneName"
 *       left press/release + double click — plays a card from hand, taps a
 *       permanent, opens the deck view entry, depending on the zone
 *   flipCard "cardName,zoneName"
 *       right-click → "Turn Over" (shorthand for cardAction)
 *   cardAction "cardName,zoneName,actionText"
 *       right-click the card and trigger the first context menu action whose
 *       label contains actionText; submenus are searched as well, so
 *       "Add counter (A)" works
 *   moveCard "cardName,srcZoneName,dstZoneName"
 *       left-drag the card onto the destination zone — plays a card, moves it
 *       to the graveyard/exile, puts it back into hand, ...
 *   drawArrow "srcCard,srcZone,dstCard,dstZone"
 *       right-drag an arrow from one card to another
 *   createToken "name,color,pt"
 *       create a token through the active local player, as the table context
 *       menu's "Create token" dialog would
 *   selectItem "item text"
 *       selects the first item of a tree/list widget whose label matches, for
 *       navigating the settings navigation tree or similar label-driven views
 *   typeText "some text"
 *       types the text into the widget one key event at a time — use it for
 *       deck lists and search boxes instead of one event per character
 *   expectProperty "propertyName,expectedValue"
 *       assertion: playback fails unless object->property(propertyName)
 *       equals expectedValue — for example "count,2" on the tab widget
 */
class pqCockatriceEventPlayer : public pqWidgetEventPlayer
{
    Q_OBJECT
    typedef pqWidgetEventPlayer Superclass;

public:
    explicit pqCockatriceEventPlayer(QObject *parent = nullptr);

    using Superclass::playEvent;

    bool playEvent(QObject *object, const QString &command, const QString &arguments, bool &error) override;

private:
    bool playClickCard(QObject *object, const QString &arguments);
    bool playFlipCard(QObject *object, const QString &arguments);
    bool playCardAction(QObject *object, const QString &arguments);
    bool playMoveCard(QObject *object, const QString &arguments);
    bool playDrawArrow(QObject *object, const QString &arguments);
    bool playCreateToken(QObject *object, const QString &arguments);
    bool playSelectItem(QObject *object, const QString &arguments);
    bool playTypeText(QObject *object, const QString &arguments);
    bool playExpectProperty(QObject *object, const QString &arguments, bool &error);

    /// Triggers the context menu entry @p actionText after right-clicking @p card.
    bool invokeCardMenuAction(const char *command, QGraphicsView *view, CardItem *card, const QString &actionText);

    static QGraphicsItem *findCardInScene(QGraphicsScene *scene, const QString &cardName, const QString &zoneName);
    static QGraphicsItem *findZoneInScene(QGraphicsScene *scene, const QString &zoneName);
    static QAction *findActionInMenu(QMenu *menu, const QString &actionText);
    static QMenu *visiblePopupMenu();

    /// Viewport position of the item's centre, as the mouse events have to be sent.
    static QPoint viewportPoint(QGraphicsView *view, const QGraphicsItem *item);
    static void sendMouseEvent(QGraphicsView *view,
                               QEvent::Type type,
                               const QPoint &pos,
                               Qt::MouseButton button,
                               Qt::MouseButtons buttons);
};

#endif // PQ_COCKATRICE_EVENT_PLAYER_H
