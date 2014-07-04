#ifndef LINEINPUT_H
#define LINEINPUT_H

#include <QLineEdit>

class QEvent;

class LineInput : public QLineEdit {
    Q_OBJECT
public:
    LineInput(QWidget *parent=0);
protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif
