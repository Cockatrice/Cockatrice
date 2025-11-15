/**
 * @file dlg_forgot_password_challenge.h
 * @ingroup AccountDialogs
 * @brief TODO: Document this.
 */

#ifndef DLG_FORGOTPASSWORDCHALLENGE_H
#define DLG_FORGOTPASSWORDCHALLENGE_H

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>

class QLabel;
class QPushButton;
class QCheckBox;

class DlgForgotPasswordChallenge : public QDialog
{
    Q_OBJECT
public:
    explicit DlgForgotPasswordChallenge(QWidget *parent = nullptr);
    [[nodiscard]] QString getHost() const
    {
        return hostEdit->text();
    }
    [[nodiscard]] int getPort() const
    {
        return portEdit->text().toInt();
    }
    [[nodiscard]] QString getPlayerName() const
    {
        return playernameEdit->text();
    }
    [[nodiscard]] QString getEmail() const
    {
        return emailEdit->text();
    }
private slots:
    void actOk();

private:
    QLabel *infoLabel, *hostLabel, *portLabel, *playernameLabel, *emailLabel;
    QLineEdit *hostEdit, *portEdit, *playernameEdit, *emailEdit;
};

#endif
