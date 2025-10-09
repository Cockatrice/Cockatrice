/**
 * @file line_edit_completer.h
 * @ingroup UI
 * @brief TODO: Document this.
 */

#ifndef LINEEDITCOMPLETER_H
#define LINEEDITCOMPLETER_H

#include "custom_line_edit.h"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QStringList>

class LineEditCompleter : public LineEditUnfocusable
{
    Q_OBJECT
private:
    QString cursorWord(const QString &line) const;
    QCompleter *c;
private slots:
    void insertCompletion(QString);

protected:
    void keyPressEvent(QKeyEvent *event);
    void focusOutEvent(QFocusEvent *e);

public:
    explicit LineEditCompleter(QWidget *parent = nullptr);
    void setCompleter(QCompleter *);
    void setCompletionList(QStringList);
};

#endif
