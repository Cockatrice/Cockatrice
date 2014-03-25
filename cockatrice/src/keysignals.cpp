#include "keysignals.h"
#include <QKeyEvent>

bool KeySignals::eventFilter(QObject * /*object*/, QEvent *event) {
    QKeyEvent *kevent;

    if(event->type() != QEvent::KeyPress)
        return false;

    kevent = static_cast<QKeyEvent *>(event);
    switch(kevent->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if(kevent->modifiers() & Qt::ControlModifier)
            emit onCtrlEnter();
        else
            emit onEnter();
        break;
    case Qt::Key_Right:
        if(kevent->modifiers() & Qt::ControlModifier)
            emit onCtrlRight();
        else
            emit onRight();

        if(!filterLROn)
            return false;
        break;
    case Qt::Key_Left:
        if(kevent->modifiers() & Qt::ControlModifier)
            emit onCtrlLeft();
        else
            emit onLeft();

        if(!filterLROn)
            return false;
        break;
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        emit onDelete();

        if(!filterDeleteOn)
            return false;
        break;
    default:
        return false;
    }

    return true;
}
