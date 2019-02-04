#include "sequenceedit.h"
#include "../settingscache.h"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QToolTip>
#include <utility>

SequenceEdit::SequenceEdit(const QString &_shortcutName, QWidget *parent) : QWidget(parent), shortcutName(_shortcutName)
{
    lineEdit = new QLineEdit(this);
    clearButton = new QPushButton("", this);
    defaultButton = new QPushButton("", this);

    lineEdit->setMinimumWidth(70);
    clearButton->setMaximumWidth(lineEdit->height());
    defaultButton->setMaximumWidth(lineEdit->height());
    clearButton->setMaximumHeight(lineEdit->height());
    defaultButton->setMaximumHeight(lineEdit->height());

    clearButton->setIcon(QPixmap("theme:icons/clearsearch"));
    defaultButton->setIcon(QPixmap("theme:icons/update"));

    clearButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    defaultButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(1);
    layout->addWidget(lineEdit);
    layout->addWidget(clearButton);
    layout->addWidget(defaultButton);

    connect(clearButton, SIGNAL(clicked()), this, SLOT(removeLastShortcut()));
    connect(defaultButton, SIGNAL(clicked()), this, SLOT(restoreDefault()));
    lineEdit->installEventFilter(this);

    lineEdit->setText(settingsCache->shortcuts().getShortcutString(shortcutName));
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
            QString valid = sequences.left(sequences.lastIndexOf(";"));
            lineEdit->setText(valid);
        } else {
            lineEdit->clear();
        }

        updateSettings();
    }
}

void SequenceEdit::restoreDefault()
{
    lineEdit->setText(settingsCache->shortcuts().getDefaultShortcutString(shortcutName));
    updateSettings();
}

void SequenceEdit::refreshShortcut()
{
    lineEdit->setText(settingsCache->shortcuts().getShortcutString(shortcutName));
}

void SequenceEdit::clear()
{
    lineEdit->setText("");
}

bool SequenceEdit::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        auto *keyEvent = reinterpret_cast<QKeyEvent *>(event);

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
        if (settingsCache->shortcuts().isKeyAllowed(shortcutName, sequenceString)) {
            if (settingsCache->shortcuts().isValid(shortcutName, sequenceString)) {
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
    settingsCache->shortcuts().setShortcuts(shortcutName, lineEdit->text());
}
