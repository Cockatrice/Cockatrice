#ifndef DLG_FORGOTPASSWORDRESET_H
#define DLG_FORGOTPASSWORDRESET_H

#include <QDialog>
#include <QLineEdit>

class QLabel;
class QPushButton;
class QCheckBox;

class DlgForgotPasswordReset : public QDialog {
    Q_OBJECT
public:
    DlgForgotPasswordReset(QString host, int port, QString playerName, QWidget *parent = 0);
    QString getHost() const { return hostEdit->text(); }
    int getPort() const { return portEdit->text().toInt(); }
	QString getPlayerName() const { return playernameEdit->text(); }
    QString getToken() const { return tokenEdit->text(); }
    QString getPassword() const { return newpasswordEdit->text(); }
private slots:
    void actOk();
    void actCancel();
private:
    QLabel *hostLabel, *portLabel, *tokenLabel, *playernameLabel, *newpasswordLabel, *confirmnewpasswordLabel;
    QLineEdit *hostEdit, *portEdit, *tokenEdit, *playernameEdit, *newpasswordEdit, *confirmnewpasswordEdit;
};

#endif
