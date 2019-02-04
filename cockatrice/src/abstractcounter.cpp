#include "abstractcounter.h"
#include "expression.h"
#include "pb/command_inc_counter.pb.h"
#include "pb/command_set_counter.pb.h"
#include "player.h"
#include "settingscache.h"
#include <QAction>
#include <QApplication>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>

AbstractCounter::AbstractCounter(Player *_player,
                                 int _id,
                                 const QString &_name,
                                 bool _shownInCounterArea,
                                 int _value,
                                 bool _useNameForShortcut,
                                 QGraphicsItem *parent)
    : QGraphicsItem(parent), player(_player), id(_id), name(_name), value(_value),
      useNameForShortcut(_useNameForShortcut), hovered(false), aDec(nullptr), aInc(nullptr), dialogSemaphore(false),
      deleteAfterDialog(false), shownInCounterArea(_shownInCounterArea)
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
            if (i == 0) {
                menu->addSeparator();
            } else {
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
        menu = nullptr;

    connect(&settingsCache->shortcuts(), SIGNAL(shortCutChanged()), this, SLOT(refreshShortcuts()));
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
    if (!player->getLocal()) {
        return;
    }
    if (name == "life") {
        shortcutActive = true;
        aSet->setShortcuts(settingsCache->shortcuts().getShortcut("Player/aSet"));
        aDec->setShortcuts(settingsCache->shortcuts().getShortcut("Player/aDec"));
        aInc->setShortcuts(settingsCache->shortcuts().getShortcut("Player/aInc"));
    } else if (useNameForShortcut) {
        shortcutActive = true;
        aSet->setShortcuts(settingsCache->shortcuts().getShortcut("Player/aSetCounter_" + name));
        aDec->setShortcuts(settingsCache->shortcuts().getShortcut("Player/aDecCounter_" + name));
        aInc->setShortcuts(settingsCache->shortcuts().getShortcut("Player/aIncCounter_" + name));
    }
}

void AbstractCounter::setShortcutsInactive()
{
    shortcutActive = false;
    if (name == "life" || useNameForShortcut) {
        aSet->setShortcut(QKeySequence());
        aDec->setShortcut(QKeySequence());
        aInc->setShortcut(QKeySequence());
    }
}

void AbstractCounter::refreshShortcuts()
{
    if (shortcutActive)
        setShortcutsActive();
}

void AbstractCounter::setValue(int _value)
{
    value = _value;
    update();
}

void AbstractCounter::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (isUnderMouse() && player->getLocal()) {
        if (event->button() == Qt::MidButton || (QApplication::keyboardModifiers() & Qt::ShiftModifier)) {
            if (menu)
                menu->exec(event->screenPos());
            event->accept();
        } else if (event->button() == Qt::LeftButton) {
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
        }
    } else
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
    QString expression = QInputDialog::getText(nullptr, tr("Set counter"), tr("New value for counter '%1':").arg(name),
                                               QLineEdit::Normal, QString::number(value), &ok);

    Expression exp(value);
    int newValue = static_cast<int>(exp.parse(expression));

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
