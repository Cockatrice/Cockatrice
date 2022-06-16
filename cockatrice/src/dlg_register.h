#ifndef DLG_REGISTER_H
#define DLG_REGISTER_H

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>

class QLabel;
class QPushButton;
class QCheckBox;

class DlgRegister : public QDialog
{
    Q_OBJECT
public:
    DlgRegister(QWidget *parent = nullptr);

private slots:
    void actOk();

private:
    QLabel *infoLabel, *hostLabel, *portLabel, *playernameLabel, *passwordLabel, *passwordConfirmationLabel,
        *emailLabel, *emailConfirmationLabel, *countryLabel, *realnameLabel;
    QLineEdit *hostEdit, *portEdit, *playernameEdit, *passwordEdit, *passwordConfirmationEdit, *emailEdit,
        *emailConfirmationEdit, *realnameEdit;
    QComboBox *countryEdit;
};

#endif
