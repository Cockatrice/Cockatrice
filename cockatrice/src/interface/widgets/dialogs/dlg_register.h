/**
 * @file dlg_register.h
 * @ingroup AccountDialogs
 * @brief TODO: Document this.
 */

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
    explicit DlgRegister(QWidget *parent = nullptr);
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
    [[nodiscard]] QString getPassword() const
    {
        return passwordEdit->text();
    }
    [[nodiscard]] QString getEmail() const
    {
        return emailEdit->text();
    }
    [[nodiscard]] QString getCountry() const
    {
        return countryEdit->currentIndex() == 0 ? "" : countryEdit->currentText();
    }
    [[nodiscard]] QString getRealName() const
    {
        return realnameEdit->text();
    }
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
