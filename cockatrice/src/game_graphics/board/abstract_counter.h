/**
 * @file abstract_counter.h
 * @ingroup GameGraphicsPlayers
 * @brief Abstract base for player counters displayed on the game board.
 */

#ifndef COUNTER_H
#define COUNTER_H

#include "../../game/board/counter_state.h"
#include "../../interface/widgets/menus/tearoff_menu.h"
#include "../player/menu/abstract_player_component.h"

#include <QGraphicsItem>
#include <QInputDialog>

class PlayerLogic;
class QAction;
class QKeyEvent;
class QMenu;
class QString;

//! \todo Document AbstractCounter class members.
class AbstractCounter : public QObject, public QGraphicsItem, public AbstractPlayerComponent
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

protected:
    PlayerLogic *player;
    int id;
    QString name;
    int value;
    QColor color;
    int radius;
    bool hovered = false;
    bool useNameForShortcut;

    /**
     * @brief Returns the counter's color adjusted for overlay rendering.
     *
     * Applies semi-transparency and lightens the color when hovered.
     * Use this for fill-based counters; SVG-based counters (GeneralCounter) bypass this.
     */
    [[nodiscard]] QColor getOverlayColor() const;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    QAction *aSet = nullptr, *aDec = nullptr, *aInc = nullptr;
    TearOffMenu *menu = nullptr;
    bool dialogSemaphore = false;
    bool deleteAfterDialog = false;
    bool shownInCounterArea;
    bool shortcutActive = false;

private slots:
    void refreshShortcuts();
    void incrementCounter();
    void setCounter();

public:
    AbstractCounter(CounterState *state,
                    PlayerLogic *player,
                    bool shownInCounterArea,
                    bool useNameForShortcut = false,
                    QGraphicsItem *parent = nullptr);
    ~AbstractCounter() override;

    void retranslateUi() override;

    /**
     * @brief Sets the counter value and triggers a visual update.
     *
     * Virtual to allow subclass display customization (e.g., CastCountWidget tooltip updates).
     * Overflow protection is handled server-side, not in client counter classes.
     *
     * @param _value The new counter value
     */
    virtual void setValue(int _value);
    void setShortcutsActive() override;
    void setShortcutsInactive() override;
    void delCounter();

    QMenu *getMenu() const
    {
        return menu;
    }
    int getId() const
    {
        return id;
    }
    QString getName() const
    {
        return name;
    }
    QColor getColor() const
    {
        return color;
    }
    int getRadius() const
    {
        return radius;
    }
    int getValue() const
    {
        return value;
    }
    bool getShownInCounterArea() const
    {
        return shownInCounterArea;
    }

    /** @brief Returns whether this counter is shown and can be modified. */
    [[nodiscard]] bool isActive() const
    {
        return active;
    }

    /**
     * @brief Shows or hides the counter and enables or disables its menu.
     *
     * Sole owner of both visibility and menu state, so container layout code only positions
     * counters and may read isActive(). Note that Qt AND-s visibility with the parent item's,
     * so an active counter nested in a hidden parent still does not render.
     *
     * @param _active True to show and enable the counter, false to hide and disable it
     */
    virtual void setActive(bool _active);

private:
    bool active = true; ///< Whether the counter is shown and modifiable
};

class AbstractCounterDialog : public QInputDialog
{
    Q_OBJECT
public:
    AbstractCounterDialog(const QString &name, const QString &value, QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void changeValue(int diff);
};

#endif
