
#include "customlineedit.h"
#include <QKeyEvent>
#include <QLineEdit>
#include <QObject>
#include <QWidget>

CustomLineEdit::CustomLineEdit(QWidget *parent) : QLineEdit(parent)
{
    installEventFilter(this);
}

CustomLineEdit::CustomLineEdit(const QString &contents, QWidget *parent) : QLineEdit(contents, parent)
{
    installEventFilter(this);
}

void CustomLineEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        clearFocus();

        return;
    }

    QLineEdit::keyPressEvent(event);
}

bool CustomLineEdit::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            event->ignore();
            return true;
        }
    }

    return QLineEdit::eventFilter(watched, event);
}
