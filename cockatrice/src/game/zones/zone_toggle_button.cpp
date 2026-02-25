#include "zone_toggle_button.h"

#include "../../client/settings/cache_settings.h"
#include "../z_values.h"
#include "command_zone.h"

#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPainterPath>

ZoneToggleButton::ZoneToggleButton(CommandZoneType _zoneType, QGraphicsItem *parent)
    : QGraphicsObject(parent), zoneType(_zoneType), expanded(false), hovered(false)
{
    setAcceptHoverEvents(true);
    setCursor(Qt::PointingHandCursor);
    setZValue(ZValues::TOGGLE_BUTTON);

    connect(&SettingsCache::instance(), &SettingsCache::langChanged, this, &ZoneToggleButton::retranslateUi);
    retranslateUi();
}

QRectF ZoneToggleButton::boundingRect() const
{
    return QRectF(0, 0, BUTTON_WIDTH, HIT_AREA_HEIGHT);
}

void ZoneToggleButton::paint(QPainter *painter,
                             [[maybe_unused]] const QStyleOptionGraphicsItem *option,
                             [[maybe_unused]] QWidget *widget)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QRectF visualRect(0, 0, BUTTON_WIDTH, BUTTON_HEIGHT);
    visualRect.adjust(0.5, 0, -0.5, -0.5);

    QPainterPath path;
    path.moveTo(visualRect.topLeft());
    path.lineTo(visualRect.topRight());
    path.lineTo(visualRect.right(), visualRect.bottom() - CORNER_RADIUS);
    path.arcTo(visualRect.right() - 2 * CORNER_RADIUS, visualRect.bottom() - 2 * CORNER_RADIUS, 2 * CORNER_RADIUS,
               2 * CORNER_RADIUS, 0, -90);
    path.lineTo(visualRect.left() + CORNER_RADIUS, visualRect.bottom());
    path.arcTo(visualRect.left(), visualRect.bottom() - 2 * CORNER_RADIUS, 2 * CORNER_RADIUS, 2 * CORNER_RADIUS, -90,
               -90);
    path.lineTo(visualRect.topLeft());
    path.closeSubpath();

    QColor bgColor = hovered ? GameColors::OVERLAY_BG_HOVERED : GameColors::OVERLAY_BG_NORMAL;

    painter->setPen(Qt::NoPen);
    painter->setBrush(bgColor);
    painter->drawPath(path);

    painter->restore();
}

void ZoneToggleButton::setExpanded(bool _expanded)
{
    if (expanded != _expanded) {
        expanded = _expanded;
        update();
    }
}

void ZoneToggleButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();
        event->accept();
    } else {
        QGraphicsObject::mousePressEvent(event);
    }
}

void ZoneToggleButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    hovered = true;
    update();
    QGraphicsObject::hoverEnterEvent(event);
}

void ZoneToggleButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    hovered = false;
    update();
    QGraphicsObject::hoverLeaveEvent(event);
}

void ZoneToggleButton::retranslateUi()
{
    switch (zoneType) {
        case CommandZoneType::Partner:
            setToolTip(tr("Click to show/hide partner zone"));
            break;
        case CommandZoneType::Companion:
            setToolTip(tr("Click to show/hide companion zone"));
            break;
        case CommandZoneType::Background:
            setToolTip(tr("Click to show/hide background zone"));
            break;
        default:
            setToolTip(tr("Click to show/hide zone"));
            break;
    }
}
