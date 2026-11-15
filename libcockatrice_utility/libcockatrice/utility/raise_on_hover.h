#ifndef COCKATRICE_RAISE_ON_HOVER_H
#define COCKATRICE_RAISE_ON_HOVER_H

#include <QObject>
#include <QPoint>

class QEvent;
class QPropertyAnimation;
class QWidget;

/**
 * @brief Raises its target widget by @p offsetPx while the mouse is over it.
 *
 * Typical use is a single-surface hover lift:
 *
 *     raiseOnHover = new RaiseOnHover(this, 10, 200);
 *     raiseOnHover->setEnabled(settings.getSelectionAnimation());
 *
 * The helper is an event filter on the target: Enter and Leave drive a pos
 * animation, Resize and Move keep the resting baseline in sync. There is no
 * Q_OBJECT macro and no signal, it is a behaviour rather than a widget; parent
 * it to the target (the default) so it dies with the widget it moves.
 *
 * Reversing mid-flight keeps the animation where it is: Enter pauses a running
 * animation and turns it forward, Leave pauses it and turns it back, and both
 * resume from the current position. A widget that never settled comes to
 * rest. A Move made while the animation runs is the animation itself, so only a
 * Move outside the animation becomes a new baseline.
 */
class RaiseOnHover : public QObject
{
public:
    explicit RaiseOnHover(QWidget *target, int offsetPx, int durationMs, QObject *parent = nullptr);

    /**
     * @brief Enables or disables the lift.
     *
     * Disabling while the target is raised leaves it where it is; the next
     * Leave settles it. That matches the historical per-widget flag, which only
     * gated future Enter / Leave handling.
     */
    void setEnabled(bool enabled)
    {
        this->enabled = enabled;
    }

    [[nodiscard]] bool isEnabled() const
    {
        return enabled;
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void raiseTarget();
    void lowerTarget();

    QWidget *target;
    QPropertyAnimation *animation;
    QPoint baseline;
    int offsetPx;
    bool enabled = true;
};

#endif // COCKATRICE_RAISE_ON_HOVER_H
