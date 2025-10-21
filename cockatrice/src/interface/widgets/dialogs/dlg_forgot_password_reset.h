/**
 * @file dlg_forgot_password_reset.h
 * @ingroup AccountDialogs
 * @brief TODO: Document this.
 */

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
    explicit DlgForgotPasswordReset(QWidget *parent = nullptr);
    QString getHost() const
    {
        return hostEdit->text();
    }
    int getPort() const
    {
        return portEdit->text().toInt();
    }
    QString getPlayerName() const
    {
        return playernameEdit->text();
    }
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
    QLabel *infoLabel, *hostLabel, *portLabel, *playernameLabel, *tokenLabel, *newpasswordLabel,
        *newpasswordverifyLabel;
    QLineEdit *hostEdit, *portEdit, *playernameEdit, *tokenEdit, *newpasswordEdit, *newpasswordverifyEdit;
};

#endif
