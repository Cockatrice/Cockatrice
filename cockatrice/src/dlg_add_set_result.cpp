#include "dlg_add_set_result.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

DlgAddSetResult::DlgAddSetResult(QWidget *parent, bool success) : QDialog(parent) {
    status = new QLabel(this);
    restart = new QLabel(this);

    if (success) {
        setWindowTitle(tr("Success"));
        status->setText(QString("Sets/cards added to Cockatrice."));
        restart->setText(QString("You must restart Cockatrice to use the new sets/cards."));
    }
    else {
        setWindowTitle(tr("Failed"));
        status->setText(QString("Sets/cards failed to import."));
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

void DlgAddSetResult::closeDialog()
{
    accept();
}