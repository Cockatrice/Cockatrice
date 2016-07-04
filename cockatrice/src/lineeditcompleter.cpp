#include <QStringListModel>
#include <QLineEdit>
#include <QCompleter>
#include <QWidget>
#include <QTextCursor>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QStringListModel>
#include <QKeyEvent>
#include <QFocusEvent>
#include "lineeditcompleter.h"

LineEditCompleter::LineEditCompleter(QWidget *parent)
: QLineEdit(parent)
{
}

void LineEditCompleter::focusOutEvent(QFocusEvent * e){
    QLineEdit::focusOutEvent(e);
    if (c->popup()->isVisible()){
        //Remove Popup
        c->popup()->hide();
        //Truncate the line to last space or whole string
        QString textValue = text();
        int lastIndex = textValue.length();
        int lastWordStartIndex = textValue.lastIndexOf(" ") + 1;
        int leftShift = qMin(lastIndex, lastWordStartIndex);
        setText(textValue.left(leftShift));
        //Insert highlighted line from popup
        insert(c->completionModel()->index(c->popup()->currentIndex().row(), 0).data().toString() + " ");
        //Set focus back to the textbox since tab was pressed
        setFocus();
    }
}

void LineEditCompleter::keyPressEvent(QKeyEvent * event)
{
    switch (event->key()){
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Escape:
        if (c->popup()->isVisible()){
            event->ignore();
            //Remove Popup
            c->popup()->hide();
            //Truncate the line to last space or whole string
            QString textValue = text();
            int lastIndexof = qMax(0, textValue.lastIndexOf(" "));
            QString finalString = textValue.left(lastIndexof);
            //Add a space if there's a word
            if (finalString != "")
                finalString += " ";
            setText(finalString);
            return;
        }
        break;
    case Qt::Key_Space:
        if (c->popup()->isVisible()){
            event->ignore();
            //Remove Popup
            c->popup()->hide();
            //Truncate the line to last space or whole string
            QString textValue = text();
            int lastIndex = textValue.length();
            int lastWordStartIndex = textValue.lastIndexOf(" ") + 1;
            int leftShift = qMin(lastIndex, lastWordStartIndex);
            setText(textValue.left(leftShift));
            //Insert highlighted line from popup
            insert(c->completionModel()->index(c->popup()->currentIndex().row(), 0).data().toString() + " ");
            return;
        }
        break;
    default:
        break;
    }

    QLineEdit::keyPressEvent(event);
    // return if the completer is null or if the most recently typed char was '@'.
    // Only want the popup AFTER typing the first char of the mention.
    if (!c || text().right(1).contains("@")) {
        c->popup()->hide();
        return;
    }

    //Set new completion prefix
    c->setCompletionPrefix(cursorWord(text()));
    if (c->completionPrefix().length() < 1){
        c->popup()->hide();
        return;
    }

    //Draw completion box
    QRect cr = cursorRect();
    cr.setWidth(c->popup()->sizeHintForColumn(0) + c->popup()->verticalScrollBar()->sizeHint().width());
    c->complete(cr);

    //Select first item in the completion popup
    QItemSelectionModel* sm = new QItemSelectionModel(c->completionModel());
    c->popup()->setSelectionModel(sm);
    sm->select(c->completionModel()->index(0, 0), QItemSelectionModel::ClearAndSelect);
    sm->setCurrentIndex(c->completionModel()->index(0, 0), QItemSelectionModel::NoUpdate);
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

void LineEditCompleter::setCompleter(QCompleter* completer)
{
    c = completer;
    c->setWidget(this);
    connect(c, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
}

void LineEditCompleter::setCompletionList(QStringList completionList)
{
    if (!c || c->popup()->isVisible())
        return;

    QStringListModel *model;
    model = (QStringListModel*)(c->model());
    if (model == NULL)
        model = new QStringListModel();
    model->setStringList(completionList);
}