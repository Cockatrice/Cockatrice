#include "commander_tax_counter.h"

#include "../../interface/theme_manager.h"
#include "../player/player.h"
#include "../player/player_actions.h"
#include "../z_values.h"

#include <QFontDatabase>
#include <QPainter>
#include <libcockatrice/protocol/pb/command_inc_counter.pb.h>

static constexpr qreal CORNER_RADIUS = 4.0;
static constexpr qreal FONT_SIZE_RATIO = 0.6;

CommanderTaxCounter::CommanderTaxCounter(Player *_player,
                                         int _id,
                                         const QString &_name,
                                         int _size,
                                         int _value,
                                         QGraphicsItem *parent)
    : AbstractCounter(_player, _id, _name, false, _value, false, parent), size(_size)
{
    // Graphics configuration must be in concrete class, not AbstractCounter.
    // Calling setCursor/setCacheMode in abstract base triggers pure virtual calls.
    // See PileZone constructor for the same pattern.
    setCacheMode(DeviceCoordinateCache);
    setAcceptHoverEvents(true);
    setCursor(Qt::ArrowCursor);

    // Update appearance when theme changes
    connect(themeManager, &ThemeManager::themeChanged, this, [this]() { update(); });

    setToolTip(tr("Commander tax: %1").arg(_value));
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

    // Draw square background with rounded corners - matches PlayerCounter style
    QRectF rect = boundingRect().adjusted(1, 1, -1, -1);

    // Use grey background color for tax counters
    QColor bgColor = hovered ? GameColors::OVERLAY_BG_HOVERED : GameColors::OVERLAY_BG_NORMAL;

    painter->setPen(Qt::NoPen);
    painter->setBrush(bgColor);
    painter->drawRoundedRect(rect, CORNER_RADIUS, CORNER_RADIUS);

    // Draw the value text using system font for cross-platform consistency
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
    // Clamp to 0 - commander tax cannot be negative
    int clampedValue = qMax(0, _value);
    AbstractCounter::setValue(clampedValue);
    setToolTip(tr("Commander tax: %1").arg(clampedValue));
}
