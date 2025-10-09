#include "abstract_counter.h"

#include "../../interface/widgets/tabs/tab_game.h"
#include "../player/player.h"
#include "translate_counter_name.h"

#include <QAction>
#include <QApplication>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QString>
#include <libcockatrice/protocol/pb/command_inc_counter.pb.h>
#include <libcockatrice/protocol/pb/command_set_counter.pb.h>
#include <libcockatrice/settings/cache_settings.h>
#include <libcockatrice/utility/expression.h>

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

    if (player->getPlayerInfo()->getLocalOrJudge()) {
        QString displayName = TranslateCounterName::getDisplayName(_name);
        menu = new TearOffMenu(displayName);
        aSet = new QAction(this);
        connect(aSet, &QAction::triggered, this, &AbstractCounter::setCounter);
        menu->addAction(aSet);
        menu->addSeparator();
        for (int i = 10; i >= -10; --i) {
            if (i == 0) {
                menu->addSeparator();
            } else {
                QAction *aIncrement = new QAction(QString(i < 0 ? "%1" : "+%1").arg(i), this);
                if (i == -1)
                    aDec = aIncrement;
                else if (i == 1)
                    aInc = aIncrement;
                aIncrement->setData(i);
                connect(aIncrement, &QAction::triggered, this, &AbstractCounter::incrementCounter);
                menu->addAction(aIncrement);
            }
        }
    } else {
        menu = nullptr;
    }

    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &AbstractCounter::refreshShortcuts);
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
    if (!player->getPlayerInfo()->getLocal()) {
        return;
    }
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();
    if (name == "life") {
        shortcutActive = true;
        aSet->setShortcuts(shortcuts.getShortcut("Player/aSet"));
        aDec->setShortcuts(shortcuts.getShortcut("Player/aDec"));
        aInc->setShortcuts(shortcuts.getShortcut("Player/aInc"));
    } else if (useNameForShortcut) {
        shortcutActive = true;
        aSet->setShortcuts(shortcuts.getShortcut("Player/aSetCounter_" + name));
        aDec->setShortcuts(shortcuts.getShortcut("Player/aDecCounter_" + name));
        aInc->setShortcuts(shortcuts.getShortcut("Player/aIncCounter_" + name));
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
    if (shortcutActive) {
        setShortcutsActive();
    }
}

void AbstractCounter::setValue(int _value)
{
    value = _value;
    update();
}

void AbstractCounter::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (isUnderMouse() && player->getPlayerInfo()->getLocalOrJudge()) {
        if (event->button() == Qt::MiddleButton || (QApplication::keyboardModifiers() & Qt::ShiftModifier)) {
            if (menu)
                menu->exec(event->screenPos());
            event->accept();
        } else if (event->button() == Qt::LeftButton) {
            Command_IncCounter cmd;
            cmd.set_counter_id(id);
            cmd.set_delta(1);
            player->getPlayerActions()->sendGameCommand(cmd);
            event->accept();
        } else if (event->button() == Qt::RightButton) {
            Command_IncCounter cmd;
            cmd.set_counter_id(id);
            cmd.set_delta(-1);
            player->getPlayerActions()->sendGameCommand(cmd);
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
    player->getPlayerActions()->sendGameCommand(cmd);
}

void AbstractCounter::setCounter()
{
    dialogSemaphore = true;
    AbstractCounterDialog dialog(name, QString::number(value), player->getGame()->getTab());
    const int ok = dialog.exec();

    if (deleteAfterDialog) {
        deleteLater();
        return;
    }
    dialogSemaphore = false;

    if (!ok)
        return;

    Expression exp(value);
    int newValue = static_cast<int>(exp.parse(dialog.textValue()));

    Command_SetCounter cmd;
    cmd.set_counter_id(id);
    cmd.set_value(newValue);
    player->getPlayerActions()->sendGameCommand(cmd);
}

AbstractCounterDialog::AbstractCounterDialog(const QString &name, const QString &value, QWidget *parent)
    : QInputDialog(parent)
{
    setWindowTitle(tr("Set counter"));
    setLabelText(tr("New value for counter '%1':").arg(name));
    setTextValue(value);
    qApp->installEventFilter(this);
}

bool AbstractCounterDialog::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
            case Qt::Key_Up:
                changeValue(+1);
                return true;
            case Qt::Key_Down:
                changeValue(-1);
                return true;
        }
    }
    return false;
}

void AbstractCounterDialog::changeValue(int diff)
{
    bool ok;
    int curValue = textValue().toInt(&ok);
    if (!ok)
        return;
    curValue += diff;
    setTextValue(QString::number(curValue));
}
