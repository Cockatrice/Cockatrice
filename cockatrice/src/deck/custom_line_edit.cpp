#include "custom_line_edit.h"

#include <QKeyEvent>
#include <QLineEdit>
#include <QObject>
#include <QTreeView>
#include <QWidget>
#include <libcockatrice/settings/cache_settings.h>
#include <libcockatrice/settings/shortcuts_settings.h>

LineEditUnfocusable::LineEditUnfocusable(QWidget *parent) : QLineEdit(parent)
{
    installEventFilter(this);
}

LineEditUnfocusable::LineEditUnfocusable(const QString &contents, QWidget *parent) : QLineEdit(contents, parent)
{
    installEventFilter(this);
}

bool LineEditUnfocusable::isUnfocusShortcut(QKeyEvent *event)
{
    QString modifier;
    QString keyNoMod;

    if (event->modifiers() & Qt::ShiftModifier)
        modifier += "Shift+";
    if (event->modifiers() & Qt::ControlModifier)
        modifier += "Ctrl+";
    if (event->modifiers() & Qt::AltModifier)
        modifier += "Alt+";
    if (event->modifiers() & Qt::MetaModifier)
        modifier += "Meta+";

    keyNoMod = QKeySequence(event->key()).toString();

    QKeySequence key(modifier + keyNoMod);
    QList<QKeySequence> unfocusShortcut = SettingsCache::instance().shortcuts().getShortcut("Player/unfocusTextBox");

    for (const auto &unfocusKey : unfocusShortcut) {
        if (key.matches(unfocusKey) == QKeySequence::ExactMatch)
            return true;
    }
    return false;
}

void LineEditUnfocusable::keyPressEvent(QKeyEvent *event)
{
    if (isUnfocusShortcut(event)) {
        clearFocus();

        return;
    }

    QLineEdit::keyPressEvent(event);
}

bool LineEditUnfocusable::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (isUnfocusShortcut(keyEvent)) {
            event->accept();

            return true;
        }
    }

    return QLineEdit::eventFilter(watched, event);
}

void SearchLineEdit::keyPressEvent(QKeyEvent *event)
{
    // List of key events that must be handled by the card list instead of the search box
    static const QVector<Qt::Key> forwardToTreeView = {Qt::Key_Up, Qt::Key_Down, Qt::Key_PageDown, Qt::Key_PageUp};
    // forward only if the search text is empty
    static const QVector<Qt::Key> forwardWhenEmpty = {Qt::Key_Home, Qt::Key_End};
    Qt::Key key = static_cast<Qt::Key>(event->key());
    if (treeView) {
        if (forwardToTreeView.contains(key))
            QCoreApplication::sendEvent(treeView, event);
        if (text().isEmpty() && forwardWhenEmpty.contains(key))
            QCoreApplication::sendEvent(treeView, event);
    }
    LineEditUnfocusable::keyPressEvent(event);
}