#ifndef DLG_ADD_SET_H
#define DLG_ADD_SET_H

#include <QDialog>
#include <QLabel>

class DlgAddSet : public QDialog {
Q_OBJECT
public:
    DlgAddSet(QWidget *parent, bool success);
private slots:
    void closeDialog();
private:
    QLabel *status, *restart;
    QPushButton *ok;
};

#endif