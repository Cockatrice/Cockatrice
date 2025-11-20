/**
 * @file line_edit_completer.h
 * @ingroup UI
 * @brief Line edit with support for multiple trigger-based completers, e.g., @mention and [[card]].
 */
//! \todo Document this file.

#ifndef LINEEDITCOMPLETER_H
#define LINEEDITCOMPLETER_H

#include "custom_line_edit.h"

#include <QCompleter>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QList>
#include <QString>
#include <QVector>

struct CompleterInfo
{
    QCompleter *completer;
    QString trigger; // e.g., "@" or "[["
};

class LineEditCompleter : public LineEditUnfocusable
{
    Q_OBJECT
signals:
    void cardPartialChanged(const QString &partial);

private:
    QList<CompleterInfo> completers;

private slots:
    void insertCompletion(const QString &text);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *e) override;

public:
    explicit LineEditCompleter(QWidget *parent = nullptr);
    void addCompleter(QCompleter *c, const QString &trigger);

    bool hasVisibleCompleterPopup() const;
    void hideCompleterPopups();
};

#endif
