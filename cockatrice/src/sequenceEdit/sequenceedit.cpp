#include "sequenceedit.h"
#include "../settingscache.h"
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QToolTip>

SequenceEdit::SequenceEdit(QString _shorcutName, QWidget *parent) : QWidget(parent)
{
    shorcutName = _shorcutName;
    currentKey = 0;
    maxKeys = 4;
    keys = 0;
    valid = false;

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

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(1);
    layout->addWidget(lineEdit);
    layout->addWidget(clearButton);
    layout->addWidget(defaultButton);

    connect(clearButton,SIGNAL(clicked()),this,SLOT(removeLastShortcut()));
    connect(defaultButton,SIGNAL(clicked()),this,SLOT(restoreDefault()));
    lineEdit->installEventFilter(this);

    lineEdit->setText(settingsCache->shortcuts().getShortcutString(shorcutName));
}

QString SequenceEdit::getSecuence()
{
    return lineEdit->text();
}

void SequenceEdit::removeLastShortcut()
{
    QString secuences = lineEdit->text();
    if(!secuences.isEmpty())
    {
        if(secuences.lastIndexOf(";") > 0){
            QString valid = secuences.left(secuences.lastIndexOf(";"));
            lineEdit->setText(valid);
        }
        else
            lineEdit->clear();
        updateSettings();
    }
}

void SequenceEdit::restoreDefault()
{
    lineEdit->setText(settingsCache->shortcuts().getDefaultShortcutString(shorcutName));
    updateSettings();
}

void SequenceEdit::refreshShortcut()
{
    lineEdit->setText(settingsCache->shortcuts().getShortcutString(shorcutName));
}

void SequenceEdit::clear()
{
    this->lineEdit->setText("");
}

bool SequenceEdit::eventFilter(QObject *, QEvent * event)
{
    if(event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
    {
        QKeyEvent * keyEvent = (QKeyEvent *) event;

        if(event->type() == QEvent::KeyPress && !keyEvent->isAutoRepeat())
            processKey(keyEvent);
        else if (event->type() == QEvent::KeyRelease && !keyEvent->isAutoRepeat())
            finishShortcut();
        return true;
    }
    return false;
}

void SequenceEdit::processKey(QKeyEvent* e)
{
    int key = e->key();
    if(key != Qt::Key_Control && key != Qt::Key_Shift
       && key != Qt::Key_Meta && key != Qt::Key_Alt)
    {
        valid = true;
        key |= translateModifiers(e->modifiers(), e->text());
    }
    keys = key;
    currentKey++;
    if(currentKey >= key)
        finishShortcut();
}

int SequenceEdit::translateModifiers(Qt::KeyboardModifiers state, const QString &text)
{
    int result = 0;
    // The shift modifier only counts when it is not used to type a symbol
    // that is only reachable using the shift key anyway
    if ((state & Qt::ShiftModifier) && (text.isEmpty() ||
                                        !text.at(0).isPrint() ||
                                        text.at(0).isLetterOrNumber() ||
                                        text.at(0).isSpace()))
        result |= Qt::SHIFT;

    if (state & Qt::ControlModifier)
        result |= Qt::CTRL;
    if (state & Qt::MetaModifier)
        result |= Qt::META;
    if (state & Qt::AltModifier)
        result |= Qt::ALT;
    return result;
}

void SequenceEdit::finishShortcut()
{
    QKeySequence secuence(keys);
    if(!secuence.isEmpty() && valid)
    {
        QString secuenceString = secuence.toString();
        if(settingsCache->shortcuts().isValid(shorcutName,secuenceString))
        {
            if(!lineEdit->text().isEmpty())
            {
                if(lineEdit->text().contains(secuenceString))
                    return;
                lineEdit->setText(lineEdit->text() + ";");
            }
            lineEdit->setText(lineEdit->text() + secuenceString);
        }
        else
        {
            QToolTip::showText(lineEdit->mapToGlobal(QPoint()), tr("Shortcut already in use"));
        }
    }
    currentKey = 0;
    keys = 0;
    valid = false;
    updateSettings();
}

void SequenceEdit::updateSettings()
{
    settingsCache->shortcuts().setShortcuts(shorcutName,lineEdit->text());
}

