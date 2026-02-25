/**
 * @file zone_toggle_button.h
 * @ingroup GameGraphicsZones
 * @brief Toggle button for showing/hiding collapsible command zones.
 */

#ifndef COCKATRICE_ZONE_TOGGLE_BUTTON_H
#define COCKATRICE_ZONE_TOGGLE_BUTTON_H

#include <QGraphicsObject>

enum class CommandZoneType;

/**
 * @class ZoneToggleButton
 * @brief Button to toggle collapsible command zone visibility.
 *
 * Rendered as a thin horizontal bar at the seam between zones.
 * Emits clicked() when activated. Supports mouse and keyboard (Enter/Space).
 *
 * Visual states: semi-transparent dark (normal), lighter (hovered).
 *
 * @see CommandZone
 */
class ZoneToggleButton : public QGraphicsObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the toggle button.
     * @param zoneType The type of zone this button toggles (used for tooltip)
     * @param parent Parent graphics item (the CommandZone)
     */
    explicit ZoneToggleButton(CommandZoneType zoneType, QGraphicsItem *parent = nullptr);
    ~ZoneToggleButton() override = default;

    [[nodiscard]] QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /**
     * @brief Updates the button's expanded state (called by parent CommandZone).
     * @param expanded True if zone is expanded
     */
    void setExpanded(bool expanded);

    /**
     * @brief Returns whether the zone is expanded.
     * @return True if expanded, false if collapsed
     */
    [[nodiscard]] bool isExpanded() const
    {
        return expanded;
    }

signals:
    void clicked();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private slots:
    void retranslateUi();

private:
    static constexpr qreal BUTTON_WIDTH = 30;
    static constexpr qreal BUTTON_HEIGHT = 5;
    static constexpr qreal HIT_AREA_HEIGHT = 15;
    static constexpr qreal CORNER_RADIUS = 1.5;
    CommandZoneType zoneType;
    bool expanded;
    bool hovered;
};

#endif // COCKATRICE_ZONE_TOGGLE_BUTTON_H
