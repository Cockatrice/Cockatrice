#include "dlg_add_set.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

DlgAddSet::DlgAddSet(QWidget *parent, bool success) : QDialog(parent) {
    status = new QLabel(this);
    restart = new QLabel(this);

    if (success) {
        setWindowTitle(tr("Success"));
        status->setText(QString("Set added to Cockatrice."));
        restart->setText(QString("You must restart Cockatrice to use the new set."));
    }
    else {
        setWindowTitle(tr("Failed"));
        status->setText(QString("Set failed to import."));
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    ok = new QPushButton(tr("Ok"), this);
    buttonBox->addButton(ok, QDialogButtonBox::AcceptRole);
    connect(ok, SIGNAL(clicked()), this, SLOT(closeDialog()));

    QVBoxLayout *parentLayout = new QVBoxLayout(this);
    parentLayout->addWidget(status);
    parentLayout->addWidget(restart);
    parentLayout->addWidget(buttonBox);

    setLayout(parentLayout);
}

void DlgAddSet::closeDialog()
{
    accept();
}