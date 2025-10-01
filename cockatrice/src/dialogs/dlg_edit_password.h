/**
 * @file dlg_edit_password.h
 * @ingroup AccountDialogs
 * @brief TODO: Document this.
 */

#ifndef DLG_EDITPASSWORD_H
#define DLG_EDITPASSWORD_H

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>

class QLabel;
class QPushButton;
class QCheckBox;

class DlgEditPassword : public QDialog
{
    Q_OBJECT
public:
    explicit DlgEditPassword(QWidget *parent = nullptr);
    QString getOldPassword() const
    {
        return oldPasswordEdit->text();
    }
    QString getNewPassword() const
    {
        return newPasswordEdit->text();
    }
private slots:
    void actOk();

private:
    QLabel *oldPasswordLabel, *newPasswordLabel, *newPasswordLabel2;
    QLineEdit *oldPasswordEdit, *newPasswordEdit, *newPasswordEdit2;
};

#endif
