#ifndef DLG_ADD_SET_RESULT_H
#define DLG_ADD_SET_RESULT_H

#include <QDialog>
#include <QLabel>
#include <QString>

class DlgAddSetResult : public QDialog {
Q_OBJECT
public:
    DlgAddSetResult(QWidget *parent, bool success, QString msg);
private slots:
    void closeDialog();
private:
    QLabel *status, *message;
    QPushButton *ok;
};

#endif