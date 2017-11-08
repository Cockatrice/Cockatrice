#include "abstractcounter.h"
#include "player.h"
#include "settingscache.h"
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include "pb/command_inc_counter.pb.h"
#include "pb/command_set_counter.pb.h"

AbstractCounter::AbstractCounter(Player *_player, int _id, const QString &_name, bool _shownInCounterArea, int _value, QGraphicsItem *parent)
    : QGraphicsItem(parent), player(_player), id(_id), name(_name), value(_value), hovered(false), aDec(0), aInc(0), dialogSemaphore(false), deleteAfterDialog(false), shownInCounterArea(_shownInCounterArea)
{
    setAcceptHoverEvents(true);

    shortcutActive = false;

    if (player->getLocal()) {
        menu = new QMenu(name);
        aSet = new QAction(this);
        connect(aSet, SIGNAL(triggered()), this, SLOT(setCounter()));
        menu->addAction(aSet);
        menu->addSeparator();
        for (int i = 10; i >= -10; --i)
            if (i == 0)
                menu->addSeparator();
            else {
                QAction *aIncrement = new QAction(QString(i < 0 ? "%1" : "+%1").arg(i), this);
                if (i == -1)
                    aDec = aIncrement;
                else if (i == 1)
                    aInc = aIncrement;
                aIncrement->setData(i);
                connect(aIncrement, SIGNAL(triggered()), this, SLOT(incrementCounter()));
                menu->addAction(aIncrement);
            }
    } else
        menu = 0;
    
    connect(&settingsCache->shortcuts(), SIGNAL(shortCutchanged()),this,SLOT(refreshShortcuts()));
    refreshShortcuts();
    retranslateUi();
}

AbstractCounter::~AbstractCounter()
{
    delete menu;
}

void AbstractCounter::delCounter()
{
    if (dialogSemaphore)
        deleteAfterDialog = true;
    else
        deleteLater();
}

void AbstractCounter::retranslateUi()
{
    if (menu) {
        aSet->setText(tr("&Set counter..."));
    }
}

void AbstractCounter::setShortcutsActive()
{
    if (name == "life") {
        shortcutActive = true;
        aSet->setShortcuts(settingsCache->shortcuts().getShortcut("Player/aSet"));
        aDec->setShortcuts(settingsCache->shortcuts().getShortcut("Player/aDec"));
        aInc->setShortcuts(settingsCache->shortcuts().getShortcut("Player/aInc"));
    }
}

void AbstractCounter::setShortcutsInactive()
{
    shortcutActive = false;
    if (name == "life") {
        aSet->setShortcut(QKeySequence());
        aDec->setShortcut(QKeySequence());
        aInc->setShortcut(QKeySequence());
    }
}

void AbstractCounter::refreshShortcuts()
{
    if(shortcutActive)
        setShortcutsActive();
}

void AbstractCounter::setValue(int _value)
{
    value = _value;
    update();
}

void AbstractCounter::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (isUnderMouse()) {
        if (event->button() == Qt::LeftButton) {
            Command_IncCounter cmd;
            cmd.set_counter_id(id);
            cmd.set_delta(1);
            player->sendGameCommand(cmd);
            event->accept();
        } else if (event->button() == Qt::RightButton) {
            Command_IncCounter cmd;
            cmd.set_counter_id(id);
            cmd.set_delta(-1);
            player->sendGameCommand(cmd);
            event->accept();
        } else if (event->button() == Qt::MidButton) {
            if (menu)
                menu->exec(event->screenPos());
            event->accept();
        } 
    }else
        event->ignore();
}

void AbstractCounter::hoverEnterEvent(QGraphicsSceneHoverEvent * /*event*/)
{
    hovered = true;
    update();
}

void AbstractCounter::hoverLeaveEvent(QGraphicsSceneHoverEvent * /*event*/)
{
    hovered = false;
    update();
}

void AbstractCounter::incrementCounter()
{
    const int delta = static_cast<QAction *>(sender())->data().toInt();
    Command_IncCounter cmd;
    cmd.set_counter_id(id);
    cmd.set_delta(delta);
    player->sendGameCommand(cmd);
}

void AbstractCounter::setCounter()
{
    bool ok;
    dialogSemaphore = true;
    int newValue = 
    QInputDialog::getInt(0, tr("Set counter"), tr("New value for counter '%1':").arg(name), value, -2000000000, 2000000000, 1, &ok);
    if (deleteAfterDialog) {
        deleteLater();
        return;
    }
    dialogSemaphore = false;
    if (!ok)
        return;
    
    Command_SetCounter cmd;
    cmd.set_counter_id(id);
    cmd.set_value(newValue);
    player->sendGameCommand(cmd);
}
