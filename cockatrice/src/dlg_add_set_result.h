#ifndef DLG_ADD_SET_RESULT_H
#define DLG_ADD_SET_RESULT_H

#include <QDialog>
#include <QLabel>

class DlgAddSetResult : public QDialog {
Q_OBJECT
public:
    DlgAddSetResult(QWidget *parent, bool success);
private slots:
    void closeDialog();
private:
    QLabel *status, *restart;
    QPushButton *ok;
};

#endif