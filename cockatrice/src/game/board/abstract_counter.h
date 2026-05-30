/**
 * @file abstract_counter.h
 * @ingroup GameGraphicsPlayers
 */
//! \todo Document this file.

#ifndef COUNTER_H
#define COUNTER_H

#include "../../interface/widgets/menus/tearoff_menu.h"
#include "../player/menu/abstract_player_component.h"
#include "counter_state.h"

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
