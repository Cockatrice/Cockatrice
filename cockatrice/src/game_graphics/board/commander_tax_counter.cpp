#include "commander_tax_counter.h"

#include "../../game/board/counter_state.h"
#include "translate_counter_name.h"

#include <QColor>
#include <QFontDatabase>
#include <QPainter>

static constexpr qreal CORNER_RADIUS = 4.0;
static constexpr qreal FONT_SIZE_RATIO = 0.6;

CommanderTaxCounter::CommanderTaxCounter(CounterState *state, PlayerLogic *player, QGraphicsItem *parent)
    : AbstractCounter(state, player, false, false, parent), size(state->getRadius())
{
    setCacheMode(DeviceCoordinateCache);
    setAcceptHoverEvents(true);
    setCursor(Qt::ArrowCursor);

    setToolTip(tr("%1: %2").arg(TranslateCounterName::getDisplayName(getName())).arg(getValue()));
}

QRectF CommanderTaxCounter::boundingRect() const
{
    return QRectF(0, 0, size, size);
}

void CommanderTaxCounter::paint(QPainter *painter,
                                [[maybe_unused]] const QStyleOptionGraphicsItem *option,
                                [[maybe_unused]] QWidget *widget)
{
    painter->save();

    QRectF rect = boundingRect().adjusted(1, 1, -1, -1);

    painter->setPen(Qt::NoPen);
    painter->setBrush(getOverlayColor());
    painter->drawRoundedRect(rect, CORNER_RADIUS, CORNER_RADIUS);

    QFont f = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    f.setPixelSize(static_cast<int>(size * FONT_SIZE_RATIO));
    f.setWeight(QFont::Bold);
    painter->setFont(f);
    painter->setPen(Qt::white);
    painter->drawText(rect, Qt::AlignCenter, QString::number(value));

    painter->restore();
}

void CommanderTaxCounter::setValue(int _value)
{
    int clampedValue = qMax(0, _value);
    AbstractCounter::setValue(clampedValue);
    setToolTip(tr("%1: %2").arg(TranslateCounterName::getDisplayName(getName())).arg(clampedValue));
}
