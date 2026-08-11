#include "cast_count_widget.h"

#include "../../game/board/counter_state.h"
#include "translate_counter_name.h"

#include <QFontDatabase>
#include <QPainter>

static constexpr qreal CORNER_RADIUS = 4.0;
static constexpr qreal FONT_SIZE_RATIO = 0.6;

CastCountWidget::CastCountWidget(CounterState *state, PlayerLogic *player, QGraphicsItem *parent)
    : AbstractCounter(state, player, false, false, parent), size(state->getRadius())
{
    setCacheMode(DeviceCoordinateCache);
    setAcceptHoverEvents(true);
    setToolTip(tr("%1: %2").arg(TranslateCounterName::getDisplayName(getName())).arg(getValue()));
}

QRectF CastCountWidget::boundingRect() const
{
    return QRectF(0, 0, size, size);
}

void CastCountWidget::paint(QPainter *painter,
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

void CastCountWidget::setValue(int _value)
{
    int clampedValue = qMax(0, _value);
    AbstractCounter::setValue(clampedValue);
    setToolTip(tr("%1: %2").arg(TranslateCounterName::getDisplayName(getName())).arg(clampedValue));
}
