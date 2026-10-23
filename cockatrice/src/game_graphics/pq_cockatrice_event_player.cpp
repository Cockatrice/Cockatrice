#include "pq_cockatrice_event_player.h"

#include "../../interface/widgets/tabs/tab_game.h"
#include "board/card_item.h"
#include "game_scene.h"
#include "game_view.h"
#include "zones/card_zone.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMenu>
#include <QMouseEvent>
#include <QTreeWidget>
#include <libcockatrice/game/abstract_game.h>
#include <libcockatrice/game/game_state.h>
#include <libcockatrice/game/player/player_actions.h>
#include <libcockatrice/game/player/player_logic.h>
#include <libcockatrice/game/player/player_manager.h>

namespace
{
/// Strips Qt's mnemonic markers so a test can name a menu entry by the label
/// the user sees ("Turn Over" rather than "T&urn Over").
QString normalizeActionText(const QString &text)
{
    QString normalized = text;
    normalized.remove('&');
    return normalized.simplified();
}

/// Maps a printable character onto the key a US keyboard would report for it,
/// so typing produces the same QKeyEvents a recording would.
Qt::Key keyForCharacter(QChar character)
{
    if (character.isLetter()) {
        return static_cast<Qt::Key>(Qt::Key_A + (character.toUpper().unicode() - 'A'));
    }
    if (character.isDigit()) {
        return static_cast<Qt::Key>(Qt::Key_0 + (character.unicode() - '0'));
    }
    switch (character.unicode()) {
        case ' ':
            return Qt::Key_Space;
        case '\n':
            return Qt::Key_Return;
        case '\t':
            return Qt::Key_Tab;
        case '-':
            return Qt::Key_Minus;
        case '_':
            return Qt::Key_Underscore;
        case '/':
            return Qt::Key_Slash;
        case ',':
            return Qt::Key_Comma;
        case '.':
            return Qt::Key_Period;
        case '\'':
            return Qt::Key_Apostrophe;
        case ';':
            return Qt::Key_Semicolon;
        case ':':
            return Qt::Key_Colon;
        case '"':
            return Qt::Key_QuoteDbl;
        case '&':
            return Qt::Key_Ampersand;
        case '(':
            return Qt::Key_ParenLeft;
        case ')':
            return Qt::Key_ParenRight;
        case '[':
            return Qt::Key_BracketLeft;
        case ']':
            return Qt::Key_BracketRight;
        default:
            return Qt::Key_unknown;
    }
}

/// Depth-first search for the first model index whose label matches @p text.
QModelIndex findModelIndexByText(const QAbstractItemModel *model, const QString &text, const QModelIndex &parent = {})
{
    for (int row = 0; row < model->rowCount(parent); ++row) {
        const QModelIndex index = model->index(row, 0, parent);
        if (index.data(Qt::DisplayRole).toString() == text) {
            return index;
        }
        const QModelIndex child = findModelIndexByText(model, text, index);
        if (child.isValid()) {
            return child;
        }
    }
    return {};
}

bool needsShift(QChar character)
{
    if (character.isUpper()) {
        return true;
    }
    return QStringLiteral("_:\"&()").contains(character);
}
} // namespace

pqCockatriceEventPlayer::pqCockatriceEventPlayer(QObject *parent) : pqWidgetEventPlayer(parent)
{
}

bool pqCockatriceEventPlayer::playEvent(QObject *object, const QString &command, const QString &arguments, bool &error)
{
    if (command == "clickCard") {
        return playClickCard(object, arguments);
    } else if (command == "flipCard") {
        return playFlipCard(object, arguments);
    } else if (command == "cardAction") {
        return playCardAction(object, arguments);
    } else if (command == "moveCard") {
        return playMoveCard(object, arguments);
    } else if (command == "drawArrow") {
        return playDrawArrow(object, arguments);
    } else if (command == "createToken") {
        return playCreateToken(object, arguments);
    } else if (command == "selectItem") {
        return playSelectItem(object, arguments);
    } else if (command == "typeText") {
        return playTypeText(object, arguments);
    } else if (command == "expectProperty") {
        return playExpectProperty(object, arguments, error);
    }
    return false;
}

QGraphicsItem *
pqCockatriceEventPlayer::findCardInScene(QGraphicsScene *scene, const QString &cardName, const QString &zoneName)
{
    for (QGraphicsItem *gi : scene->items()) {
        if (gi->type() != typeCard) {
            continue;
        }
        auto *card = static_cast<CardItem *>(gi);
        if (!card->getZone() || card->getZone()->getName() != zoneName) {
            continue;
        }
        QString actualName = card->getName();
        if (actualName == cardName || actualName.endsWith(" " + cardName)) {
            return card;
        }
    }
    return nullptr;
}

QGraphicsItem *pqCockatriceEventPlayer::findZoneInScene(QGraphicsScene *scene, const QString &zoneName)
{
    for (QGraphicsItem *gi : scene->items()) {
        if (gi->type() != typeZone) {
            continue;
        }
        auto *zone = static_cast<CardZone *>(gi);
        if (zone->getLogic() && zone->getLogic()->getName() == zoneName) {
            return zone;
        }
    }
    return nullptr;
}

QAction *pqCockatriceEventPlayer::findActionInMenu(QMenu *menu, const QString &actionText)
{
    if (!menu || actionText.isEmpty()) {
        return nullptr;
    }
    for (QAction *action : menu->actions()) {
        if (action->isSeparator()) {
            continue;
        }
        if (normalizeActionText(action->text()).contains(actionText, Qt::CaseInsensitive)) {
            return action;
        }
        if (QMenu *subMenu = action->menu()) {
            if (QAction *found = findActionInMenu(subMenu, actionText)) {
                return found;
            }
        }
    }
    return nullptr;
}

QMenu *pqCockatriceEventPlayer::visiblePopupMenu()
{
    for (QWidget *w : QApplication::topLevelWidgets()) {
        auto *menu = qobject_cast<QMenu *>(w);
        if (menu && menu->isVisible()) {
            return menu;
        }
    }
    return nullptr;
}

QPoint pqCockatriceEventPlayer::viewportPoint(QGraphicsView *view, const QGraphicsItem *item)
{
    return view->mapFromScene(item->sceneBoundingRect().center());
}
void pqCockatriceEventPlayer::sendMouseEvent(QGraphicsView *view,
                                             QEvent::Type type,
                                             const QPoint &pos,
                                             Qt::MouseButton button,
                                             Qt::MouseButtons buttons)
{
    QMouseEvent event(type, pos, view->viewport()->mapToGlobal(pos), button, buttons, Qt::NoModifier);
    qApp->notify(view->viewport(), &event);
}

bool pqCockatriceEventPlayer::playClickCard(QObject *object, const QString &arguments)
{
    auto *gv = qobject_cast<QGraphicsView *>(object);
    if (!gv) {
        qCritical() << "clickCard: target widget is not a QGraphicsView";
        return false;
    }

    QStringList parts = arguments.split(',');
    if (parts.size() < 2) {
        qCritical() << "clickCard: arguments must be 'cardName,zoneName', got:" << arguments;
        return false;
    }

    QGraphicsScene *scene = gv->scene();
    if (!scene) {
        qCritical() << "clickCard: no scene on the QGraphicsView";
        return false;
    }

    auto *target = findCardInScene(scene, parts[0].trimmed(), parts[1].trimmed());
    if (!target) {
        qCritical() << "clickCard: card" << parts[0].trimmed() << "not found in zone" << parts[1].trimmed();
        return false;
    }

    const QPoint pos = viewportPoint(gv, target);
    sendMouseEvent(gv, QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton);
    sendMouseEvent(gv, QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton);
    sendMouseEvent(gv, QEvent::MouseButtonDblClick, pos, Qt::LeftButton, Qt::LeftButton);
    sendMouseEvent(gv, QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton);

    return true;
}

bool pqCockatriceEventPlayer::playFlipCard(QObject *object, const QString &arguments)
{
    auto *gv = qobject_cast<QGraphicsView *>(object);
    if (!gv) {
        qCritical() << "flipCard: target widget is not a QGraphicsView";
        return false;
    }

    QStringList parts = arguments.split(',');
    if (parts.size() < 2) {
        qCritical() << "flipCard: arguments must be 'cardName,zoneName', got:" << arguments;
        return false;
    }

    QGraphicsScene *scene = gv->scene();
    if (!scene) {
        qCritical() << "flipCard: no scene";
        return false;
    }

    auto *card = qgraphicsitem_cast<CardItem *>(findCardInScene(scene, parts[0].trimmed(), parts[1].trimmed()));
    if (!card) {
        qCritical() << "flipCard: card" << parts[0].trimmed() << "not found in zone" << parts[1].trimmed();
        return false;
    }

    return invokeCardMenuAction("flipCard", gv, card, QStringLiteral("Turn Over"));
}

bool pqCockatriceEventPlayer::playCardAction(QObject *object, const QString &arguments)
{
    auto *gv = qobject_cast<QGraphicsView *>(object);
    if (!gv) {
        qCritical() << "cardAction: target widget is not a QGraphicsView";
        return false;
    }

    QStringList parts = arguments.split(',');
    if (parts.size() < 3) {
        qCritical() << "cardAction: arguments must be 'cardName,zoneName,actionText', got:" << arguments;
        return false;
    }

    QGraphicsScene *scene = gv->scene();
    if (!scene) {
        qCritical() << "cardAction: no scene";
        return false;
    }

    auto *card = qgraphicsitem_cast<CardItem *>(findCardInScene(scene, parts[0].trimmed(), parts[1].trimmed()));
    if (!card) {
        qCritical() << "cardAction: card" << parts[0].trimmed() << "not found in zone" << parts[1].trimmed();
        return false;
    }

    return invokeCardMenuAction("cardAction", gv, card, parts.mid(2).join(',').trimmed());
}

bool pqCockatriceEventPlayer::invokeCardMenuAction(const char *command,
                                                   QGraphicsView *view,
                                                   CardItem *card,
                                                   const QString &actionText)
{
    const QPoint pos = viewportPoint(view, card);
    sendMouseEvent(view, QEvent::MouseButtonPress, pos, Qt::RightButton, Qt::RightButton);
    sendMouseEvent(view, QEvent::MouseButtonRelease, pos, Qt::RightButton, Qt::NoButton);
    qApp->processEvents();

    QMenu *menu = visiblePopupMenu();
    if (!menu) {
        qWarning() << command << ": no popup menu appeared after right-click";
        return false;
    }

    QAction *action = findActionInMenu(menu, actionText);
    if (!action) {
        qWarning() << command << ": no context menu action matching" << actionText;
        menu->close();
        return false;
    }

    action->trigger();
    menu->close();
    qApp->processEvents();
    return true;
}

bool pqCockatriceEventPlayer::playMoveCard(QObject *object, const QString &arguments)
{
    auto *gv = qobject_cast<QGraphicsView *>(object);
    if (!gv) {
        qCritical() << "moveCard: target widget is not a QGraphicsView";
        return false;
    }

    QStringList parts = arguments.split(',');
    if (parts.size() < 3) {
        qCritical() << "moveCard: arguments must be 'cardName,srcZoneName,dstZoneName', got:" << arguments;
        return false;
    }

    const QString cardName = parts[0].trimmed();
    const QString srcZoneName = parts[1].trimmed();
    const QString dstZoneName = parts[2].trimmed();

    QGraphicsScene *scene = gv->scene();
    if (!scene) {
        qCritical() << "moveCard: no scene";
        return false;
    }

    auto *card = findCardInScene(scene, cardName, srcZoneName);
    if (!card) {
        qCritical() << "moveCard: card" << cardName << "not found in zone" << srcZoneName;
        return false;
    }

    auto *dstZone = findZoneInScene(scene, dstZoneName);
    if (!dstZone) {
        qCritical() << "moveCard: destination zone" << dstZoneName << "not found";
        return false;
    }

    const QPoint srcPos = viewportPoint(gv, card);
    const QPoint dstPos = viewportPoint(gv, dstZone);

    // A drag has to travel past Qt's start distance before the scene turns it
    // into a card drag item, otherwise the move is interpreted as a click.
    const int dragDistance = 2 * QApplication::startDragDistance() + 10;
    const QPoint pastDragStart = srcPos + QPoint(dragDistance, 0);

    sendMouseEvent(gv, QEvent::MouseButtonPress, srcPos, Qt::LeftButton, Qt::LeftButton);
    sendMouseEvent(gv, QEvent::MouseMove, pastDragStart, Qt::LeftButton, Qt::LeftButton);
    qApp->processEvents();
    sendMouseEvent(gv, QEvent::MouseMove, dstPos, Qt::LeftButton, Qt::LeftButton);
    qApp->processEvents();
    sendMouseEvent(gv, QEvent::MouseButtonRelease, dstPos, Qt::LeftButton, Qt::NoButton);
    qApp->processEvents();

    return true;
}

bool pqCockatriceEventPlayer::playDrawArrow(QObject *object, const QString &arguments)
{
    auto *gv = qobject_cast<QGraphicsView *>(object);
    if (!gv) {
        qCritical() << "drawArrow: target widget is not a QGraphicsView";
        return false;
    }

    QStringList parts = arguments.split(',');
    if (parts.size() < 4) {
        qCritical() << "drawArrow: arguments must be 'srcCard,srcZone,dstCard,dstZone', got:" << arguments;
        return false;
    }

    const QString srcCardName = parts[0].trimmed();
    const QString srcZoneName = parts[1].trimmed();
    const QString dstCardName = parts[2].trimmed();
    const QString dstZoneName = parts[3].trimmed();

    QGraphicsScene *scene = gv->scene();
    if (!scene) {
        qCritical() << "drawArrow: no scene";
        return false;
    }

    auto *srcCard = findCardInScene(scene, srcCardName, srcZoneName);
    if (!srcCard) {
        qCritical() << "drawArrow: source card" << srcCardName << "not found in zone" << srcZoneName;
        return false;
    }

    auto *dstCard = findCardInScene(scene, dstCardName, dstZoneName);
    if (!dstCard) {
        qCritical() << "drawArrow: dest card" << dstCardName << "not found in zone" << dstZoneName;
        return false;
    }

    const QPoint srcPos = viewportPoint(gv, srcCard);
    const QPoint dstPos = viewportPoint(gv, dstCard);

    const int dragDistance = 2 * QApplication::startDragDistance() + 10;
    const QPoint pastDragStart = srcPos + QPoint(dragDistance, 0);

    sendMouseEvent(gv, QEvent::MouseButtonPress, srcPos, Qt::RightButton, Qt::RightButton);
    sendMouseEvent(gv, QEvent::MouseMove, pastDragStart, Qt::RightButton, Qt::RightButton);
    qApp->processEvents();
    sendMouseEvent(gv, QEvent::MouseMove, dstPos, Qt::RightButton, Qt::RightButton);
    sendMouseEvent(gv, QEvent::MouseButtonRelease, dstPos, Qt::RightButton, Qt::NoButton);
    qApp->processEvents();

    return true;
}

bool pqCockatriceEventPlayer::playCreateToken(QObject *object, const QString &arguments)
{
    auto *gv = qobject_cast<QGraphicsView *>(object);
    if (!gv) {
        qCritical() << "createToken: target widget is not a QGraphicsView";
        return false;
    }

    QStringList parts = arguments.split(',');
    if (parts.size() < 3) {
        qCritical() << "createToken: arguments must be 'name,color,pt', got:" << arguments;
        return false;
    }

    QString tokenName = parts[0].trimmed();
    QString tokenColor = parts[1].trimmed();
    QString tokenPt = parts[2].trimmed();

    TabGame *tabGame = nullptr;
    QWidget *w = gv;
    while (w) {
        tabGame = qobject_cast<TabGame *>(w);
        if (tabGame) {
            break;
        }
        w = w->parentWidget();
    }
    if (!tabGame) {
        qCritical() << "createToken: TabGame not found";
        return false;
    }

    AbstractGame *game = tabGame->getGame();
    if (!game) {
        qCritical() << "createToken: no game";
        return false;
    }

    int activePlayerId = game->getGameState()->getActivePlayer();
    PlayerLogic *player = game->getPlayerManager()->getActiveLocalPlayer(activePlayerId);
    if (!player) {
        qCritical() << "createToken: no active local player";
        return false;
    }

    TokenInfo info;
    info.name = tokenName;
    info.color = tokenColor;
    info.pt = tokenPt;
    info.destroy = true;
    info.faceDown = false;

    player->getPlayerActions()->actCreateToken(info);

    for (int i = 0; i < 10; ++i) {
        qApp->processEvents();
    }

    return true;
}

bool pqCockatriceEventPlayer::playSelectItem(QObject *object, const QString &arguments)
{
    const QString text = arguments.trimmed();
    if (text.isEmpty()) {
        qCritical() << "selectItem: needs the label of the item to select";
        return false;
    }

    if (auto *tree = qobject_cast<QTreeWidget *>(object)) {
        QTreeWidgetItemIterator item(tree);
        while (*item) {
            if ((*item)->text(0) == text) {
                tree->setCurrentItem(*item);
                qApp->processEvents();
                return true;
            }
            ++item;
        }
        qCritical() << "selectItem: no tree item labelled" << text;
        return false;
    }

    if (auto *view = qobject_cast<QAbstractItemView *>(object)) {
        const QModelIndex index = findModelIndexByText(view->model(), text);
        if (!index.isValid()) {
            qCritical() << "selectItem: no item labelled" << text;
            return false;
        }
        view->setCurrentIndex(index);
        qApp->processEvents();
        return true;
    }

    qCritical() << "selectItem: target is neither a tree widget nor an item view";
    return false;
}

bool pqCockatriceEventPlayer::playTypeText(QObject *object, const QString &arguments)
{
    auto *widget = qobject_cast<QWidget *>(object);
    if (!widget) {
        qCritical() << "typeText: target is not a QWidget";
        return false;
    }

    for (const QChar character : arguments) {
        const Qt::Key key = keyForCharacter(character);
        const Qt::KeyboardModifiers modifiers = needsShift(character) ? Qt::ShiftModifier : Qt::NoModifier;
        QKeyEvent press(QEvent::KeyPress, key, modifiers, QString(character));
        QKeyEvent release(QEvent::KeyRelease, key, modifiers, QString(character));
        qApp->notify(widget, &press);
        qApp->notify(widget, &release);
    }
    qApp->processEvents();

    return true;
}

bool pqCockatriceEventPlayer::playExpectProperty(QObject *object, const QString &arguments, bool &error)
{
    if (!object) {
        qCritical() << "expectProperty: no target object";
        error = true;
        return true;
    }

    QStringList parts = arguments.split(',');
    if (parts.size() < 2) {
        qCritical() << "expectProperty: arguments must be 'propertyName,expectedValue', got:" << arguments;
        error = true;
        return true;
    }

    const QString propertyName = parts[0].trimmed();
    const QString expected = parts.mid(1).join(',');

    const QVariant actual = object->property(propertyName.toUtf8().constData());
    if (!actual.isValid()) {
        qCritical() << "expectProperty:" << object->objectName() << "has no property" << propertyName;
        error = true;
        return true;
    }

    if (actual.toString() != expected) {
        qCritical() << "expectProperty:" << object->objectName() << propertyName << "is" << actual.toString()
                    << "but the test expected" << expected;
        error = true;
        return true;
    }

    return true;
}
