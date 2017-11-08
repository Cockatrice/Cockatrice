#ifndef COUNTER_H
#define COUNTER_H

#include <QGraphicsItem>

class Player;
class QMenu;
class QAction;

class AbstractCounter : public QObject, public QGraphicsItem {
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
protected:
    Player *player;
    int id;
    QString name;
    int value;
    bool hovered;
    
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
private:
    QAction *aSet, *aDec, *aInc;
    QMenu *menu;
    bool dialogSemaphore, deleteAfterDialog;
    bool shownInCounterArea;
private slots:
    void refreshShortcuts();
    void incrementCounter();
    void setCounter();
public:
    AbstractCounter(Player *_player, int _id, const QString &_name, bool _shownInCounterArea, int _value, QGraphicsItem *parent = 0);
    ~AbstractCounter();
    
    QMenu *getMenu() const { return menu; }
    void retranslateUi();
    
    int getId() const { return id; }
    QString getName() const { return name; }
    bool getShownInCounterArea() const { return shownInCounterArea; }
    int getValue() const { return value; }
    void setValue(int _value);
    void delCounter();
    
    void setShortcutsActive();
    void setShortcutsInactive();
    bool shortcutActive;
};

#endif
