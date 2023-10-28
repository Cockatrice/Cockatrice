#include "sequenceedit.h"

#include "../settingscache.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QToolTip>
#include <utility>

SequenceEdit::SequenceEdit(const QString &_shortcutName, QWidget *parent) : QWidget(parent)
{
    lineEdit = new QLineEdit(this);
    clearButton = new QPushButton("", this);
    defaultButton = new QPushButton("", this);

    lineEdit->setMinimumWidth(70);
    clearButton->setIcon(QPixmap("theme:icons/clearsearch"));
    defaultButton->setIcon(QPixmap("theme:icons/update"));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(1);
    layout->addWidget(lineEdit);
    layout->addWidget(clearButton);
    layout->addWidget(defaultButton);

    connect(clearButton, SIGNAL(clicked()), this, SLOT(removeLastShortcut()));
    connect(defaultButton, SIGNAL(clicked()), this, SLOT(restoreDefault()));
    lineEdit->installEventFilter(this);

    setShortcutName(_shortcutName);
    retranslateUi();
}

void SequenceEdit::setShortcutName(const QString &_shortcutName)
{
    shortcutName = _shortcutName;
    if (shortcutName.isEmpty()) {
        clearButton->setEnabled(false);
        defaultButton->setEnabled(false);
        lineEdit->setEnabled(false);
        lineEdit->setText("");
        lineEdit->setPlaceholderText(tr("Choose an action from the table"));
    } else {
        clearButton->setEnabled(true);
        defaultButton->setEnabled(true);
        lineEdit->setEnabled(true);
        lineEdit->setText(SettingsCache::instance().shortcuts().getShortcutString(shortcutName));
        lineEdit->setPlaceholderText(tr("Hit the key/combination of keys you want to set for this action"));
    }
}

QString SequenceEdit::getSequence()
{
    return lineEdit->text();
}

void SequenceEdit::removeLastShortcut()
{
    QString sequences = lineEdit->text();
    if (!sequences.isEmpty()) {
        if (sequences.lastIndexOf(";") > 0) {
            QString validText = sequences.left(sequences.lastIndexOf(";"));
            lineEdit->setText(validText);
        } else {
            lineEdit->clear();
        }

        updateSettings();
    }
}

void SequenceEdit::restoreDefault()
{
    lineEdit->setText(SettingsCache::instance().shortcuts().getDefaultShortcutString(shortcutName));
    updateSettings();
}

void SequenceEdit::refreshShortcut()
{
    lineEdit->setText(SettingsCache::instance().shortcuts().getShortcutString(shortcutName));
}

void SequenceEdit::clear()
{
    lineEdit->setText("");
}

bool SequenceEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        auto *keyEvent = reinterpret_cast<QKeyEvent *>(event);

        // don't filter outside arrow key events
        if (obj != lineEdit) {
            switch (keyEvent->key()) {
                case Qt::Key_Up:
                case Qt::Key_Down:
                case Qt::Key_Left:
                case Qt::Key_Right:
                    return false;
                default:
                    break;
            }
        }
        if (event->type() == QEvent::KeyPress && !keyEvent->isAutoRepeat()) {
            processKey(keyEvent);
        } else if (event->type() == QEvent::KeyRelease && !keyEvent->isAutoRepeat()) {
            finishShortcut();
        }

        return true;
    }
    return false;
}

void SequenceEdit::processKey(QKeyEvent *e)
{
    int key = e->key();
    if (key != Qt::Key_Control && key != Qt::Key_Shift && key != Qt::Key_Meta && key != Qt::Key_Alt) {
        valid = true;
        key |= translateModifiers(e->modifiers(), e->text());
    }

    keys = key;
    currentKey++;
    if (currentKey >= key) {
        finishShortcut();
    }
}

int SequenceEdit::translateModifiers(Qt::KeyboardModifiers state, const QString &text)
{
    int result = 0;
    // The shift modifier only counts when it is not used to type a symbol
    // that is only reachable using the shift key anyway
    if ((state & Qt::ShiftModifier) &&
        (text.isEmpty() || !text.at(0).isPrint() || text.at(0).isLetterOrNumber() || text.at(0).isSpace())) {
        result |= Qt::SHIFT;
    }

    if (state & Qt::ControlModifier) {
        result |= Qt::CTRL;
    }

    if (state & Qt::MetaModifier) {
        result |= Qt::META;
    }

    if (state & Qt::AltModifier) {
        result |= Qt::ALT;
    }

    return result;
}

void SequenceEdit::finishShortcut()
{
    QKeySequence sequence(keys);
    if (!sequence.isEmpty() && valid) {
        QString sequenceString = sequence.toString();
        if (SettingsCache::instance().shortcuts().isKeyAllowed(shortcutName, sequenceString)) {
            if (SettingsCache::instance().shortcuts().isValid(shortcutName, sequenceString)) {
                if (!lineEdit->text().isEmpty()) {
                    if (lineEdit->text().contains(sequenceString)) {
                        return;
                    }
                    lineEdit->setText(lineEdit->text() + ";");
                }
                lineEdit->setText(lineEdit->text() + sequenceString);
            } else {
                QToolTip::showText(lineEdit->mapToGlobal(QPoint()), tr("Shortcut already in use"));
            }
        } else {
            QToolTip::showText(lineEdit->mapToGlobal(QPoint()), tr("Invalid key"));
        }
    }

    currentKey = 0;
    keys = 0;
    valid = false;
    updateSettings();
}

void SequenceEdit::updateSettings()
{
    SettingsCache::instance().shortcuts().setShortcuts(shortcutName, lineEdit->text());
}

void SequenceEdit::retranslateUi()
{
    clearButton->setText(tr("Clear"));
    defaultButton->setText(tr("Restore default"));
    setShortcutName(shortcutName);
}
