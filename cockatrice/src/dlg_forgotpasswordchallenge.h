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
    DlgForgotPasswordChallenge(QWidget *parent = 0);
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
    QString getEmail() const
    {
        return emailEdit->text();
    }
private slots:
    void actOk();

private:
    QLabel *hostLabel, *portLabel, *playernameLabel, *emailLabel;
    QLineEdit *hostEdit, *portEdit, *playernameEdit, *emailEdit;
};

#endif
