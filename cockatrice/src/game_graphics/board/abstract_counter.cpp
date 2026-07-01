#include "abstract_counter.h"

#include "../../client/settings/cache_settings.h"
#include "../../game/player/player_actions.h"
#include "../../game/player/player_logic.h"
#include "../../game_graphics/board/translate_counter_name.h"
#include "../../interface/widgets/tabs/tab_game.h"

#include <QAction>
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMenu>
#include <QString>
#include <libcockatrice/protocol/pb/command_inc_counter.pb.h>
#include <libcockatrice/protocol/pb/command_set_counter.pb.h>
#include <libcockatrice/utility/expression.h>

AbstractCounter::AbstractCounter(CounterState *state,
                                 PlayerLogic *_player,
                                 bool _shownInCounterArea,
                                 bool _useNameForShortcut,
                                 QGraphicsItem *parent)
    : QGraphicsItem(parent), player(_player), id(state->getId()), name(state->getName()), value(state->getValue()),
      color(state->getColor()), radius(state->getRadius()), useNameForShortcut(_useNameForShortcut),
      shownInCounterArea(_shownInCounterArea)
{
    setAcceptHoverEvents(true);

    // Route through the (possibly overridden) virtual setValue so subclasses such as
    // CommanderTaxCounter can clamp and refresh their tooltip on every value change.
    connect(state, &CounterState::valueChanged, this, [this](int, int newValue) { setValue(newValue); });

    connect(state, &CounterState::activeChanged, this, [this](bool newActive) {
        setActive(newActive);
        emit player->rearrangeCounters();
    });

    if (player->getPlayerInfo()->getLocalOrJudge()) {
        menu = new TearOffMenu(TranslateCounterName::getDisplayName(state->getName()));
        aSet = new QAction(this);
        connect(aSet, &QAction::triggered, this, &AbstractCounter::setCounter);
        menu->addAction(aSet);
        menu->addSeparator();
        for (int i = 10; i >= -10; --i) {
            if (i == 0) {
                menu->addSeparator();
                continue;
            }
            auto *a = new QAction(QString(i < 0 ? "%1" : "+%1").arg(i), this);
            if (i == -1) {
                aDec = a;
            }
            if (i == 1) {
                aInc = a;
            }
            a->setData(i);
            connect(a, &QAction::triggered, this, &AbstractCounter::incrementCounter);
            menu->addAction(a);
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
    if (dialogSemaphore) {
        deleteAfterDialog = true;
    } else {
        deleteLater();
    }
}

void AbstractCounter::setValue(int _value)
{
    value = _value;
    update();
}

void AbstractCounter::setActive(bool _active)
{
    active = _active;
    setVisible(_active);
    update();
}

void AbstractCounter::retranslateUi()
{
    if (aSet) {
        aSet->setText(tr("&Set counter..."));
    }
}

void AbstractCounter::setShortcutsActive()
{
    if (!menu || !player->getPlayerInfo()->getLocal()) {
        return;
    }
    ShortcutsSettings &sc = SettingsCache::instance().shortcuts();
    shortcutActive = true;
    if (name == "life") {
        aSet->setShortcuts(sc.getShortcut("Player/aSet"));
        aDec->setShortcuts(sc.getShortcut("Player/aDec"));
        aInc->setShortcuts(sc.getShortcut("Player/aInc"));
    } else if (useNameForShortcut) {
        aSet->setShortcuts(sc.getShortcut("Player/aSetCounter_" + name));
        aDec->setShortcuts(sc.getShortcut("Player/aDecCounter_" + name));
        aInc->setShortcuts(sc.getShortcut("Player/aIncCounter_" + name));
    }
}

void AbstractCounter::setShortcutsInactive()
{
    if (!menu) {
        return;
    }

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

void AbstractCounter::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!isUnderMouse() || !player->getPlayerInfo()->getLocalOrJudge()) {
        event->ignore();
        return;
    }

    if (event->button() == Qt::MiddleButton || QApplication::keyboardModifiers() & Qt::ShiftModifier) {
        if (menu) {
            menu->exec(event->screenPos());
        }
    } else {
        Command_IncCounter cmd;
        cmd.set_counter_id(id);
        cmd.set_delta(event->button() == Qt::LeftButton ? 1 : -1);
        player->getPlayerActions()->sendGameCommand(cmd);
    }
    event->accept();
}

void AbstractCounter::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    hovered = true;
    update();
}
void AbstractCounter::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    hovered = false;
    update();
}

void AbstractCounter::incrementCounter()
{
    Command_IncCounter cmd;
    cmd.set_counter_id(id);
    cmd.set_delta(static_cast<QAction *>(sender())->data().toInt());
    player->getPlayerActions()->sendGameCommand(cmd);
}

void AbstractCounter::setCounter()
{
    QWidget *parent = nullptr;
    if (auto *view = scene() ? scene()->views().value(0) : nullptr) {
        parent = view->window();
    }

    dialogSemaphore = true;
    AbstractCounterDialog dlg(name, QString::number(value), parent);
    const int ok = dlg.exec();
    dialogSemaphore = false;

    if (deleteAfterDialog) {
        deleteLater();
        return;
    }
    if (!ok) {
        return;
    }

    Expression exp(value);
    Command_SetCounter cmd;
    cmd.set_counter_id(id);
    cmd.set_value(static_cast<int>(exp.parse(dlg.textValue())));
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
    if (!ok) {
        return;
    }
    curValue += diff;
    setTextValue(QString::number(curValue));
}
