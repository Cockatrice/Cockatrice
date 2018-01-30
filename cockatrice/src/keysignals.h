#ifndef KEYSIGNALS_H
#define KEYSIGNALS_H

#include <QEvent>
#include <QObject>

class KeySignals : public QObject
{
    Q_OBJECT

signals:
    void onEnter();
    void onCtrlEnter();
    void onCtrlAltEnter();
    void onLeft();
    void onRight();
    void onDelete();
    void onCtrlAltMinus();
    void onCtrlAltEqual();
    void onCtrlAltLBracket();
    void onCtrlAltRBracket();
    void onS();

protected:
    virtual bool eventFilter(QObject *, QEvent *event);
};

#endif
