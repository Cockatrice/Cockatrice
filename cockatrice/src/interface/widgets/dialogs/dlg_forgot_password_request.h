/**
 * @file dlg_forgot_password_request.h
 * @ingroup AccountDialogs
 * @brief TODO: Document this.
 */

#ifndef DLG_FORGOTPASSWORDREQUEST_H
#define DLG_FORGOTPASSWORDREQUEST_H

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>

class QLabel;
class QPushButton;
class QCheckBox;

class DlgForgotPasswordRequest : public QDialog
{
    Q_OBJECT
public:
    explicit DlgForgotPasswordRequest(QWidget *parent = nullptr);
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
private slots:
    void actOk();

private:
    QLabel *infoLabel, *hostLabel, *portLabel, *playernameLabel;
    QLineEdit *hostEdit, *portEdit, *playernameEdit;
};

#endif
