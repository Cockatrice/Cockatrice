#include <QKeyEvent>
#include <QKeySequence>
#include <QDebug>
#include "lineinput.h"

LineInput::LineInput(QWidget *parent)
    : QLineEdit(parent)
{
}

bool LineInput::eventFilter(QObject *obj, QEvent *event)
{
    // intercept keyborad events from chatView
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = (QKeyEvent*)event;

        // ensure "copy" and "select all" shortcuts works on the chatView
        if(!(keyEvent->key() == Qt::Key_Control) &&
            !keyEvent->matches(QKeySequence::Copy) &&
          !keyEvent->matches(QKeySequence::SelectAll))
        {
            setFocus();
            keyPressEvent(keyEvent);
            return true;
        }
    }

    return QLineEdit::eventFilter(obj, event);
}
