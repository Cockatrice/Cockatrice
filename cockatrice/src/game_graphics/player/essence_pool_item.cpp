#include "essence_pool_item.h"

#include "../../interface/pixel_map_generator.h"

#include <QFont>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QPainter>
#include <QPen>
#include <QRectF>
#include <QSize>
#include <QWidget>
#include <libcockatrice/game/board/counter_state.h>
#include <libcockatrice/game/player/player_actions.h>
#include <libcockatrice/game/player/player_info.h>
#include <libcockatrice/game/player/player_logic.h>
#include <libcockatrice/utility/essence_counter.h>

#include <limits>

EssencePoolItem::EssencePoolItem(PlayerLogic *_player, QGraphicsItem *parent)
    : QGraphicsObject(parent), player(_player)
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setAcceptHoverEvents(true);
    setZValue(5);

    connect(player, &PlayerLogic::counterAdded, this, &EssencePoolItem::attachCounter);
    connect(player, &PlayerLogic::counterRemoved, this, &EssencePoolItem::detachCounter);

    // Reconnect and late view construction must paint the full snapshot,
    // even when the counterAdded events happened before this item existed.
    for (CounterState *state : player->getCounters()) {
        attachCounter(state);
    }
    setToolTip(tr("Essence pool — left click +1, right click -1, Shift+left to set"));
}

QRectF EssencePoolItem::boundingRect() const
{
    return QRectF(0, 0, Width, Height);
}

void EssencePoolItem::attachCounter(CounterState *state)
{
    if (!state) {
        return;
    }
    const QChar code = EssenceCounter::codeFromName(state->getName());
    if (code.isNull()) {
        return;
    }
    counters.insert(code, state);
    connect(state, &CounterState::valueChanged, this, [this](int, int, bool) { update(); });
    update();
}

void EssencePoolItem::detachCounter(int id)
{
    for (auto it = counters.begin(); it != counters.end();) {
        if (it.value() && it.value()->getId() == id) {
            it = counters.erase(it);
        } else {
            ++it;
        }
    }
    update();
}

CounterState *EssencePoolItem::stateFor(QChar code) const
{
    return counters.value(code).data();
}

QChar EssencePoolItem::codeAt(const QPointF &position) const
{
    const qreal x = position.x() - Padding;
    const qreal y = position.y() - HeaderHeight;
    if (x < 0 || y < 0 || x >= Columns * CellWidth || y >= 4 * CellHeight) {
        return QChar();
    }
    const int col = static_cast<int>(x) / CellWidth;
    const int row = static_cast<int>(y) / CellHeight;
    const int index = row * Columns + col;
    const QString &codes = EssenceCounter::symbols();
    return index >= 0 && index < codes.size() ? codes.at(index) : QChar();
}

void EssencePoolItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(QColor(118, 125, 158), 1));
    painter->setBrush(QColor(16, 19, 31, 232));
    painter->drawRoundedRect(boundingRect().adjusted(0.5, 0.5, -0.5, -0.5), 7, 7);

    QFont titleFont = painter->font();
    titleFont.setBold(true);
    titleFont.setPixelSize(15);
    painter->setFont(titleFont);
    painter->setPen(QColor(236, 238, 248));
    painter->drawText(QRectF(8, 3, Width - 16, 24), Qt::AlignCenter, tr("ESSENCE POOL"));

    QFont countFont = painter->font();
    countFont.setBold(true);
    countFont.setPixelSize(14);
    painter->setFont(countFont);

    const QString &codes = EssenceCounter::symbols();
    for (int i = 0; i < codes.size(); ++i) {
        const QChar code = codes.at(i);
        const int col = i % Columns;
        const int row = i / Columns;
        const qreal x = Padding + col * CellWidth;
        const qreal y = HeaderHeight + row * CellHeight;
        const QRectF tile(x, y, CellWidth - 3, CellHeight - 3);

        painter->setPen(QPen(QColor(85, 91, 119), 1));
        painter->setBrush(QColor(43, 47, 67, 228));
        painter->drawRoundedRect(tile, 5, 5);

        const QPixmap icon = ManaSymbolPixmapGenerator::generatePixmap(QString(code), QSize(32, 32));
        if (!icon.isNull()) {
            painter->drawPixmap(QPointF(x + 9, y + 2), icon);
        } else {
            painter->setPen(Qt::white);
            painter->drawText(QRectF(x, y + 2, CellWidth - 3, 30), Qt::AlignCenter, QString(code));
        }

        CounterState *state = stateFor(code);
        const int count = state ? state->getValue() : 0;
        painter->setPen(count > 0 ? QColor(255, 244, 191) : QColor(178, 182, 200));
        painter->drawText(QRectF(x, y + 33, CellWidth - 3, 18),
                          Qt::AlignCenter, QString::number(count));
    }

    painter->restore();
}

void EssencePoolItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    const QChar code = codeAt(event->pos());
    CounterState *state = stateFor(code);
    if (!state || !player->getPlayerInfo()->getLocalOrJudge()) {
        event->ignore();
        return;
    }
    PlayerActions *actions = player->getPlayerActions();
    if (event->button() == Qt::LeftButton && (event->modifiers() & Qt::ShiftModifier)) {
        const QString label = code == QChar('N') ? tr("Colorless (N)") :
                              code == QChar('J') ? tr("Jasper (J)") : tr("Essence %1").arg(code);
        bool accepted = false;
        QWidget *parent = event->widget() ? event->widget()->window() : nullptr;
        const int value = QInputDialog::getInt(parent, tr("Set Essence"),
                                              tr("Available %1:").arg(label),
                                              state->getValue(), 0, std::numeric_limits<int>::max(), 1, &accepted);
        if (accepted) {
            actions->actSetCounter(state->getId(), value);
        }
    } else if (event->button() == Qt::LeftButton) {
        actions->actIncrementCounter(state->getId(), 1);
    } else if (event->button() == Qt::RightButton) {
        if (state->getValue() > 0) {
            actions->actIncrementCounter(state->getId(), -1);
        }
    } else {
        event->ignore();
        return;
    }
    event->accept();
}

void EssencePoolItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    const QChar code = codeAt(event->pos());
    CounterState *state = stateFor(code);
    if (!state) {
        setToolTip(tr("Essence pool — all types start at zero"));
        return;
    }
    const QString name = code == QChar('N') ? tr("Colorless Essence") :
                         code == QChar('J') ? tr("Jasper Essence") : tr("Essence %1").arg(code);
    setToolTip(tr("%1: %2 available\nLeft +1 | Right -1 | Shift+Left: Set")
                   .arg(name).arg(state->getValue()));
}
