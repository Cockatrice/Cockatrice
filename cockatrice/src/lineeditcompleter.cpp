#include "lineeditcompleter.h"
#include <QAbstractItemView>
#include <QCompleter>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QScrollBar>
#include <QStringListModel>
#include <QTextCursor>
#include <QWidget>

LineEditCompleter::LineEditCompleter(QWidget *parent) : QLineEdit(parent)
{
}

void LineEditCompleter::focusOutEvent(QFocusEvent *e)
{
    QLineEdit::focusOutEvent(e);
    if (mentionCompleter->popup()->isVisible()) {
        // Remove Popup
        mentionCompleter->popup()->hide();
        // Truncate the line to last space or whole string
        QString textValue = text();
        int lastIndex = textValue.length();
        int lastWordStartIndex = textValue.lastIndexOf(" ") + 1;
        int leftShift = qMin(lastIndex, lastWordStartIndex);
        setText(textValue.left(leftShift));
        // Insert highlighted line from popup
        insert(mentionCompleter->completionModel()
                   ->index(mentionCompleter->popup()->currentIndex().row(), 0)
                   .data()
                   .toString() +
               " ");
        // Set focus back to the textbox since tab was pressed
        setFocus();
    }

    if (cardCompleter->popup()->isVisible()) {
        // Remove Popup
        cardCompleter->popup()->hide();
        // Truncate the line to last space or whole string
        QString textValue = text();
        int lastIndex = textValue.length();
        int lastWordStartIndex = textValue.lastIndexOf(" ") + 1;
        int leftShift = qMin(lastIndex, lastWordStartIndex);
        setText(textValue.left(leftShift));
        // Insert highlighted line from popup
        insert(
            cardCompleter->completionModel()->index(cardCompleter->popup()->currentIndex().row(), 0).data().toString() +
            " ");
        // Set focus back to the textbox since tab was pressed
        setFocus();
    }
}

void LineEditCompleter::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Escape:
            if (mentionCompleter->popup()->isVisible()) {
                event->ignore();
                // Remove Popup
                mentionCompleter->popup()->hide();
                // Truncate the line to last space or whole string
                QString textValue = text();
                int lastIndexof = qMax(0, textValue.lastIndexOf(" "));
                QString finalString = textValue.left(lastIndexof);
                // Add a space if there's a word
                if (finalString != "")
                    finalString += " ";
                setText(finalString);
                return;
            }
            if (cardCompleter->popup()->isVisible()) {
                event->ignore();
                // Remove Popup
                cardCompleter->popup()->hide();
                // Truncate the line to last space or whole string
                QString textValue = text();
                int lastIndexof = qMax(0, textValue.lastIndexOf(" "));
                QString finalString = textValue.left(lastIndexof);
                // Add a space if there's a word
                if (finalString != "")
                    finalString += " ";
                setText(finalString);
                return;
            }
            break;
        case Qt::Key_Space:
            if (mentionCompleter->popup()->isVisible()) {
                event->ignore();
                // Remove Popup
                mentionCompleter->popup()->hide();
                // Truncate the line to last space or whole string
                QString textValue = text();
                int lastIndex = textValue.length();
                int lastWordStartIndex = textValue.lastIndexOf(" ") + 1;
                int leftShift = qMin(lastIndex, lastWordStartIndex);
                setText(textValue.left(leftShift));
                // Insert highlighted line from popup
                insert(mentionCompleter->completionModel()
                           ->index(mentionCompleter->popup()->currentIndex().row(), 0)
                           .data()
                           .toString() +
                       " ");
                return;
            }
            if (cardCompleter->popup()->isVisible()) {
                event->ignore();
                // Remove Popup
                cardCompleter->popup()->hide();
                // Truncate the line to last space or whole string
                QString textValue = text();
                int lastIndex = textValue.length();
                int lastWordStartIndex = textValue.lastIndexOf(" ") + 1;
                int leftShift = qMin(lastIndex, lastWordStartIndex);
                setText(textValue.left(leftShift));
                // Insert highlighted line from popup
                insert(cardCompleter->completionModel()
                           ->index(cardCompleter->popup()->currentIndex().row(), 0)
                           .data()
                           .toString() +
                       " ");
                return;
            }
            break;
        default:
            break;
    }

    QLineEdit::keyPressEvent(event);

    // return if the mentionCompleter is null or if the most recently typed char was '@'.
    // Only want the popup AFTER typing the first char of the mention.
    if (mentionCompleter && !text().right(1).contains("@")) {
        processMention();
    } else {
        mentionCompleter->popup()->hide();
    }

    if (cardCompleter && !text().right(2).contains("[[")) {
        processCard();
    } else {
        cardCompleter->popup()->hide();
    }

    return;
}

void LineEditCompleter::processMention() const
{
    // Set new completion prefix
    mentionCompleter->setCompletionPrefix(cursorWord(text()));
    if (mentionCompleter->completionPrefix().length() < 1) {
        mentionCompleter->popup()->hide();
        return;
    }

    // Select first item in the completion popup
    QItemSelectionModel *sm = new QItemSelectionModel(mentionCompleter->completionModel());
    mentionCompleter->popup()->setSelectionModel(sm);
    sm->select(mentionCompleter->completionModel()->index(0, 0), QItemSelectionModel::ClearAndSelect);
    sm->setCurrentIndex(mentionCompleter->completionModel()->index(0, 0), QItemSelectionModel::NoUpdate);
}

void LineEditCompleter::processCard() const
{
    cardCompleter->setCompletionPrefix(cursorWord(text()));
    if (cardCompleter->completionPrefix().length() < 2) {
        cardCompleter->popup()->hide();
        return;
    }

    // Draw completion box
    QRect cr = cursorRect();
    cr.setWidth(cardCompleter->popup()->sizeHintForColumn(0) +
                cardCompleter->popup()->verticalScrollBar()->sizeHint().width());
    cardCompleter->complete(cr);

    // Select first item in the completion popup
    QItemSelectionModel *sm = new QItemSelectionModel(cardCompleter->completionModel());
    cardCompleter->popup()->setSelectionModel(sm);
    sm->select(cardCompleter->completionModel()->index(0, 0), QItemSelectionModel::ClearAndSelect);
    sm->setCurrentIndex(cardCompleter->completionModel()->index(0, 0), QItemSelectionModel::NoUpdate);
}

QString LineEditCompleter::cursorWord(const QString &line) const
{
    return line.mid(line.left(cursorPosition()).lastIndexOf(" ") + 1,
                    cursorPosition() - line.left(cursorPosition()).lastIndexOf(" ") - 1);
}

void LineEditCompleter::insertCompletion(QString arg)
{
    QString s_arg = arg + " ";
    setText(text().replace(text().left(cursorPosition()).lastIndexOf(" ") + 1,
                           cursorPosition() - text().left(cursorPosition()).lastIndexOf(" ") - 1, s_arg));
}

void LineEditCompleter::setMentionCompleter(QCompleter *completer)
{
    mentionCompleter = completer;
    mentionCompleter->setWidget(this);
    connect(mentionCompleter, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
}

void LineEditCompleter::setCardCompleter(QCompleter *completer)
{
    cardCompleter = completer;
    cardCompleter->setWidget(this);
    connect(cardCompleter, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
}

void LineEditCompleter::setMentionCompletionList(QStringList completionList)
{
    if (!mentionCompleter || mentionCompleter->popup()->isVisible())
        return;

    QStringListModel *model;
    model = (QStringListModel *)(mentionCompleter->model());
    if (model == NULL)
        model = new QStringListModel();
    model->setStringList(completionList);
}