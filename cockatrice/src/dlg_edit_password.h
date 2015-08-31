#ifndef DLG_EDITPASSWORD_H
#define DLG_EDITPASSWORD_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>

class QLabel;
class QPushButton;
class QCheckBox;

class DlgEditPassword : public QDialog {
    Q_OBJECT
public:
    DlgEditPassword(QWidget *parent = 0);
    QString getOldPassword() const { return oldPasswordEdit->text(); }
    QString getNewPassword() const { return newPasswordEdit->text(); }
private slots:
    void actOk();
    void actCancel();
private:
    QLabel *oldPasswordLabel, *newPasswordLabel, *newPasswordLabel2;
    QLineEdit *oldPasswordEdit, *newPasswordEdit, *newPasswordEdit2;
};

#endif
