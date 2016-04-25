#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QDebug>

#include "dlg_edit_user.h"
#include "settingscache.h"

DlgEditUser::DlgEditUser(QWidget *parent, QString email, QString country, QString realName)
    : QDialog(parent)
{
    emailLabel = new QLabel(tr("Email:"));
    emailEdit = new QLineEdit();
    emailLabel->setBuddy(emailEdit);
    emailEdit->setText(email);

    countryLabel = new QLabel(tr("Country:"));
    countryEdit = new QComboBox();
    countryLabel->setBuddy(countryEdit);
    countryEdit->insertItem(0, tr("Undefined"));
    countryEdit->setCurrentIndex(0);

    QStringList countries = settingsCache->getCountries();
    int i = 1;
    foreach(QString c, countries)
    {
        countryEdit->addItem(QPixmap("theme:countries/" + c.toLower()), c);
        if (c == country)
            countryEdit->setCurrentIndex(i);

        ++i;
    }

    realnameLabel = new QLabel(tr("Real name:"));
    realnameEdit = new QLineEdit();
    realnameLabel->setBuddy(realnameEdit);
    realnameEdit->setText(realName);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(emailLabel, 0, 0);
    grid->addWidget(emailEdit, 0, 1);
    grid->addWidget(countryLabel, 2, 0);
    grid->addWidget(countryEdit, 2, 1);
    grid->addWidget(realnameLabel, 3, 0);
    grid->addWidget(realnameEdit, 3, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(actCancel()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Edit user profile"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);
}

void DlgEditUser::actOk()
{
    accept();
}

void DlgEditUser::actCancel()
{
    reject();
}
