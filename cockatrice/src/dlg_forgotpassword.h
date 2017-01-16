#ifndef DLG_FORGOTPASSWORD_H
#define DLG_FORGOTPASSWORD_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>

class QLabel;
class QPushButton;
class QCheckBox;

class DlgForgotPassword : public QDialog {
    Q_OBJECT
public:
    DlgForgotPassword(QWidget *parent = 0);
    QString getHost() const { return hostEdit->text(); }
    int getPort() const { return portEdit->text().toInt(); }
    QString getPlayerName() const { return playernameEdit->text(); }
    QString getEmail() const { return emailEdit->text(); }
private slots:
    void actOk();
    void actCancel();
private:
    QLabel *hostLabel, *portLabel, *playernameLabel, *emailLabel;
    QLineEdit *hostEdit, *portEdit, *playernameEdit, *emailEdit;
    QComboBox *countryEdit;
};

#endif
