#include "line_edit_completer.h"

#include <QAbstractItemView>
#include <QFocusEvent>
#include <QKeyEvent>

LineEditCompleter::LineEditCompleter(QWidget *parent) : LineEditUnfocusable(parent)
{
}

void LineEditCompleter::addCompleter(QCompleter *c, const QString &trigger)
{
    c->setWidget(this);
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    connect(c, qOverload<const QString &>(&QCompleter::activated), this, &LineEditCompleter::insertCompletion);

    completers.append({c, trigger});
}

bool LineEditCompleter::hasVisibleCompleterPopup() const
{
    for (const auto &info : completers) {
        if (info.completer->popup()->isVisible()) {
            return true;
        }
    }
    return false;
}

void LineEditCompleter::hideCompleterPopups()
{
    for (const auto &info : completers) {
        info.completer->popup()->hide();
    }
}

void LineEditCompleter::focusOutEvent(QFocusEvent *e)
{
    LineEditUnfocusable::focusOutEvent(e);

    hideCompleterPopups();
}

void LineEditCompleter::keyPressEvent(QKeyEvent *event)
{
    LineEditUnfocusable::keyPressEvent(event);

    QString textValue = text();
    int cursorPos = cursorPosition();

    CompleterInfo *active = nullptr;
    QString prefix;

    for (auto &info : completers) {
        if (info.trigger == "@") {
            int triggerPos = textValue.lastIndexOf("@", cursorPos - 1);
            if (triggerPos != -1 && (triggerPos == 0 || textValue[triggerPos - 1].isSpace())) {
                active = &info;
                prefix = textValue.mid(triggerPos + 1, cursorPos - (triggerPos + 1));
                break;
            }
        } else if (info.trigger == "[[") {
            int triggerPos = textValue.lastIndexOf("[[", cursorPos - 1);
            int closePos = textValue.indexOf("]]", triggerPos + 2);
            if (triggerPos != -1 && (closePos == -1 || closePos >= cursorPos)) {
                active = &info;
                prefix = textValue.mid(triggerPos + 2, cursorPos - (triggerPos + 2));
                break;
            }
        }
    }

    if (!active) {
        for (auto &info : completers) {
            info.completer->popup()->hide();
        }
        return;
    }

    active->completer->setCompletionPrefix(prefix);

    if (active->trigger == "[[") {
        emit cardPartialChanged(prefix);
        return;
    }

    active->completer->complete();
}

void LineEditCompleter::insertCompletion(const QString &completion)
{
    QString t = text();
    int pos = cursorPosition();

    CompleterInfo *active = nullptr;
    for (auto &info : completers) {
        if (info.completer == sender()) {
            active = &info;
            break;
        }
    }
    if (!active) {
        return;
    }

    if (active->trigger == "[[") {
        int triggerPos = t.lastIndexOf("[[", pos - 1);
        if (triggerPos == -1) {
            return;
        }
        QString after = t.mid(pos);
        QString replaced = t.left(triggerPos + 2) + completion + "]] ";
        setText(replaced + after);
        setCursorPosition(replaced.length());
        return;
    }

    int triggerPos = t.lastIndexOf("@", pos - 1);
    if (triggerPos == -1) {
        return;
    }
    setText(t.replace(triggerPos, pos - triggerPos, completion + " "));
    setCursorPosition(triggerPos + completion.length() + 1);
}
