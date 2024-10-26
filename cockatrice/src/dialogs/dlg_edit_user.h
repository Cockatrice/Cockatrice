#ifndef DLG_EDITUSER_H
#define DLG_EDITUSER_H

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>

class QLabel;
class QPushButton;
class QCheckBox;

class DlgEditUser : public QDialog
{
    Q_OBJECT
public:
    DlgEditUser(QWidget *parent = nullptr,
                QString email = QString(),
                QString country = QString(),
                QString realName = QString());
    QString getEmail() const
    {
        return emailEdit->text();
    }
    QString getCountry() const
    {
        return countryEdit->currentIndex() == 0 ? "" : countryEdit->currentText();
    }
    QString getRealName() const
    {
        return realnameEdit->text();
    }
private slots:
    void actOk();

private:
    QLabel *emailLabel, *countryLabel, *realnameLabel;
    QLineEdit *emailEdit, *realnameEdit;
    QComboBox *countryEdit;
};

#endif
