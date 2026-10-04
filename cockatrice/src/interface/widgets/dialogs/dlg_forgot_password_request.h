/**
 * @file dlg_forgot_password_request.h
 * @ingroup AccountDialogs
 */
//! \todo Document this file.

#ifndef DLG_FORGOTPASSWORDREQUEST_H
#define DLG_FORGOTPASSWORDREQUEST_H

#include <QDialog>
#include <QLineEdit>
#include <QString>
#include <qtmetamacros.h>

class QLabel;
class QWidget;

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
