/**
 * @file abstract_counter.h
 * @ingroup GameGraphicsPlayers
 * @brief Abstract base for player counters displayed on the game board.
 */
//! \todo Document this file.

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
     * Virtual to allow subclass display customization (e.g., CommanderTaxCounter tooltip updates).
     * Overflow protection is handled server-side, not in client counter classes.
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

    /**
     * @brief Returns whether this counter is active (visible and interactable).
     * Inactive counters are hidden and their menu actions should be disabled.
     */
    [[nodiscard]] bool isActive() const
    {
        return active;
    }

    /**
     * @brief Sets the active state of this counter.
     *
     * This is the sole owner of the counter's own visibility flag: active maps directly to
     * setVisible(_active). For counters nested in another item (e.g. tax counters parented to
     * the command zone), Qt AND-s this flag with the parent's visibility, so an active counter
     * inside a hidden zone still does not render. Container layout code must therefore not set
     * visibility itself; it only positions counters and may read isActive().
     *
     * @param _active True to show and enable the counter, false to hide it
     */
    virtual void setActive(bool _active);

private:
    bool active = true;
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
