#ifndef COUNTER_H
#define COUNTER_H

#include <QGraphicsItem>
#include <QInputDialog>

class Player;
class QAction;
class QKeyEvent;
class QMenu;
class QString;

class AbstractCounter : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

protected:
    Player *player;
    int id;
    QString name;
    int value;
    bool useNameForShortcut, hovered;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    QAction *aSet, *aDec, *aInc;
    QMenu *menu;
    bool dialogSemaphore, deleteAfterDialog;
    bool shownInCounterArea;
    bool shortcutActive;

private slots:
    void refreshShortcuts();
    void incrementCounter();
    void setCounter();
    void setCounterAccepted(QString expression);

public:
    AbstractCounter(Player *_player,
                    int _id,
                    const QString &_name,
                    bool _shownInCounterArea,
                    int _value,
                    bool _useNameForShortcut = false,
                    QGraphicsItem *parent = nullptr);
    ~AbstractCounter() override;

    void retranslateUi();
    void setValue(int _value);
    void setShortcutsActive();
    void setShortcutsInactive();
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
    bool getShownInCounterArea() const
    {
        return shownInCounterArea;
    }
    int getValue() const
    {
        return value;
    }
};

class AbstractCounterDialog: public QInputDialog
{
    Q_OBJECT
public:
    AbstractCounterDialog(const QString &name, const QString &value);
protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void changeValue(int diff);
};

#endif
