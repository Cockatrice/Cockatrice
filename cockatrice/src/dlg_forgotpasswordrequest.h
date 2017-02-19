#ifndef DLG_FORGOTPASSWORDREQUEST_H
#define DLG_FORGOTPASSWORDREQUEST_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>

class QLabel;
class QPushButton;
class QCheckBox;

class DlgForgotPasswordRequest : public QDialog {
    Q_OBJECT
public:
    DlgForgotPasswordRequest(QWidget *parent = 0);
    QString getHost() const { return hostEdit->text(); }
    int getPort() const { return portEdit->text().toInt(); }
    QString getPlayerName() const { return playernameEdit->text(); }
private slots:
    void actOk();
    void actCancel();
private:
    QLabel *hostLabel, *portLabel, *playernameLabel;
    QLineEdit *hostEdit, *portEdit, *playernameEdit;
};

#endif
