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
    DlgForgotPasswordChallenge(QWidget *parent = nullptr);
    QString getEmail() const
    {
        return emailEdit->text();
    }
private slots:
    void actOk();

private:
    QLabel *infoLabel, *emailLabel;
    QLineEdit *emailEdit;
};

#endif
