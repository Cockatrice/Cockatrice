#ifndef DLG_REGISTER_H
#define DLG_REGISTER_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>

class QLabel;
class QPushButton;
class QCheckBox;

class DlgRegister : public QDialog {
    Q_OBJECT
public:
    DlgRegister(QWidget *parent = 0);
    QString getHost() const { return hostEdit->currentText(); }
    int getPort() const { return portEdit->text().toInt(); }
    QString getPlayerName() const { return playernameEdit->text(); }
    QString getPassword() const { return passwordEdit->text(); }
    QString getEmail() const { return emailEdit->text(); }
    int getGender() const { return genderEdit->currentIndex() - 1; }
    QString getCountry() const { return countryEdit->currentIndex() == 0 ? "" : countryEdit->currentText(); }
    QString getRealName() const { return realnameEdit->text(); }
private slots:
    void actOk();
    void actCancel();
private:
    QLabel *hostLabel, *portLabel, *playernameLabel, *passwordLabel, *passwordConfirmationLabel, *emailLabel, *emailConfirmationLabel, *genderLabel, *countryLabel, *realnameLabel;
    QLineEdit *portEdit, *playernameEdit, *passwordEdit, *passwordConfirmationEdit, *emailEdit, *emailConfirmationEdit, *realnameEdit;
    QComboBox *hostEdit, *genderEdit, *countryEdit;
};

#endif
