#ifndef DLG_FORGOTPASSWORDRESET_H
#define DLG_FORGOTPASSWORDRESET_H

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>

class QLabel;
class QPushButton;
class QCheckBox;

class DlgForgotPasswordReset : public QDialog
{
    Q_OBJECT
public:
    DlgForgotPasswordReset(QWidget *parent = nullptr);
    QString getToken() const
    {
        return tokenEdit->text();
    }
    QString getPassword() const
    {
        return newpasswordEdit->text();
    }
private slots:
    void actOk();

private:
    QLabel *infoLabel, *tokenLabel, *newpasswordLabel, *newpasswordverifyLabel;
    QLineEdit *tokenEdit, *newpasswordEdit, *newpasswordverifyEdit;
};

#endif
