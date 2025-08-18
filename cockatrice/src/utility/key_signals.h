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
    void onShiftLeft();
    void onShiftRight();
    void onDelete();
    void onCtrlAltMinus();
    void onCtrlAltEqual();
    void onCtrlAltLBracket();
    void onCtrlAltRBracket();
    void onShiftS();
    void onCtrlC();

protected:
    bool eventFilter(QObject *, QEvent *event) override;
};

#endif
