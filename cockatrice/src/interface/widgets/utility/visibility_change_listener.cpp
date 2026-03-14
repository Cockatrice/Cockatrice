#include "visibility_change_listener.h"

#include <QEvent>
#include <QWidget>

VisibilityChangeListener::VisibilityChangeListener(QWidget *targetWidget)
    : QObject(targetWidget), targetWidget(targetWidget)
{
    if (targetWidget) {
        targetWidget->installEventFilter(this);
    }
}

bool VisibilityChangeListener::eventFilter(QObject *o, QEvent *e)
{
    if (o == targetWidget && !e->spontaneous()) {
        if (e->type() == QEvent::Show) {
            emit visibilityChanged(true);
        }

        if (e->type() == QEvent::Hide) {
            emit visibilityChanged(false);
        }
    }
    return false;
}