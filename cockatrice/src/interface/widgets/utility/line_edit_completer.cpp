#include "line_edit_completer.h"

#include <QAbstractItemView>
#include <QFocusEvent>
#include <QKeyEvent>

LineEditCompleter::LineEditCompleter(QWidget *parent) : LineEditUnfocusable(parent)
{
}

void LineEditCompleter::addCompleter(QCompleter *c, CompleterTrigger trigger)
{
    c->setWidget(this);
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    connect(c, qOverload<const QString &>(&QCompleter::activated), this,
            qOverload<const QString &>(&LineEditCompleter::insertCompletion));

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

    // Only commit the highlighted completion when focus moves away via Tab.
    // Other focus losses (e.g. the unfocus shortcut / Escape) must simply close
    // the popup without inserting anything.
    if (e->reason() != Qt::TabFocusReason) {
        hideCompleterPopups();
        return;
    }

    for (auto &info : completers) {
        if (!info.completer->popup()->isVisible()) {
            continue;
        }

        const QModelIndex currentIndex = info.completer->popup()->currentIndex();
        if (currentIndex.isValid()) {
            insertCompletion(info.completer, currentIndex.data().toString());
        }
    }

    hideCompleterPopups();
}

void LineEditCompleter::keyPressEvent(QKeyEvent *event)
{
    LineEditUnfocusable::keyPressEvent(event);

    if (event->key() == Qt::Key_Escape) {
        hideCompleterPopups();
        return;
    }

    QString textValue = text();
    int cursorPos = cursorPosition();

    CompleterInfo *active = nullptr;
    QString prefix;

    for (auto &info : completers) {
        bool triggered = false;
        switch (info.trigger) {
            case CompleterTrigger::Mention: {
                int triggerPos = textValue.lastIndexOf("@", cursorPos - 1);
                if (triggerPos != -1 && (triggerPos == 0 || textValue[triggerPos - 1].isSpace())) {
                    triggered = true;
                    // Keep the "@" so the prefix matches the "@"-prefixed mention model entries.
                    prefix = textValue.mid(triggerPos, cursorPos - triggerPos);
                }
                break;
            }
            case CompleterTrigger::Card: {
                int triggerPos = textValue.lastIndexOf("[[", cursorPos - 1);
                int closePos = textValue.indexOf("]]", triggerPos + 2);
                if (triggerPos != -1 && (closePos == -1 || closePos >= cursorPos)) {
                    triggered = true;
                    prefix = textValue.mid(triggerPos + 2, cursorPos - (triggerPos + 2));
                }
                break;
            }
        }

        if (triggered) {
            active = &info;
            break;
        }
    }

    if (!active) {
        for (auto &info : completers) {
            info.completer->popup()->hide();
        }
        return;
    }

    active->completer->setCompletionPrefix(prefix);

    switch (active->trigger) {
        case CompleterTrigger::Card:
            emit cardPartialChanged(prefix);
            return;
        case CompleterTrigger::Mention:
            break;
    }

    active->completer->complete();
}

void LineEditCompleter::insertCompletion(const QString &completion)
{
    for (auto &info : completers) {
        if (info.completer == sender()) {
            insertCompletion(info.completer, completion);
            return;
        }
    }
}

void LineEditCompleter::insertCompletion(QCompleter *completer, const QString &completion)
{
    QString t = text();
    int pos = cursorPosition();

    for (const auto &info : completers) {
        if (info.completer != completer) {
            continue;
        }

        switch (info.trigger) {
            case CompleterTrigger::Card: {
                int triggerPos = t.lastIndexOf("[[", pos - 1);
                if (triggerPos == -1) {
                    return;
                }

                // If an earlier "[[" is still open it also encloses the cursor, so
                // replace from its start. Otherwise completing in text such as
                // "[[Opt[[Amok" would leave a stray "[[" behind.
                int startPos = triggerPos;
                for (int searchFrom = triggerPos; searchFrom > 0;) {
                    const int earlier = t.lastIndexOf("[[", searchFrom - 1);
                    if (earlier == -1) {
                        break;
                    }
                    const int earlierClose = t.indexOf("]]", earlier + 2);
                    if (earlierClose != -1 && earlierClose < pos) {
                        break;
                    }
                    startPos = earlier;
                    searchFrom = earlier;
                }

                // If the cursor sits inside an already-closed [[...]] pair, replace
                // the whole construct instead of leaving a duplicate closing bracket
                // behind.
                int insertEnd = pos;
                const int closePos = t.indexOf("]]", startPos + 2);
                if (closePos != -1 && closePos >= pos) {
                    insertEnd = closePos + 2;
                }

                QString after = t.mid(insertEnd);
                QString replaced = t.left(startPos + 2) + completion + "]] ";
                setText(replaced + after);
                setCursorPosition(replaced.length());
                return;
            }
            case CompleterTrigger::Mention: {
                int triggerPos = t.lastIndexOf("@", pos - 1);
                if (triggerPos == -1) {
                    return;
                }
                setText(t.replace(triggerPos, pos - triggerPos, completion + " "));
                setCursorPosition(triggerPos + completion.length() + 1);
                return;
            }
        }
    }
}
