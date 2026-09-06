#include "account_setup_page.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

AccountSetupPage::AccountSetupPage(QWidget *parent) : FirstRunWizardPage(parent)
{
    bodyLabel = new QLabel(this);
    bodyLabel->setWordWrap(true);
    bodyLabel->setAlignment(Qt::AlignCenter);

    registerButton = new QPushButton(this);
    connectButton = new QPushButton(this);
    skipHintLabel = new QLabel(this);
    skipHintLabel->setWordWrap(true);
    skipHintLabel->setAlignment(Qt::AlignCenter);

    connect(registerButton, &QPushButton::clicked, this, &AccountSetupPage::registerRequested);
    connect(connectButton, &QPushButton::clicked, this, &AccountSetupPage::connectRequested);

    auto *layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(bodyLabel);
    layout->addSpacing(16);
    layout->addWidget(registerButton, 0, Qt::AlignHCenter);
    layout->addWidget(connectButton, 0, Qt::AlignHCenter);
    layout->addSpacing(16);
    layout->addWidget(skipHintLabel);
    layout->addStretch();

    retranslateUi();
}

bool AccountSetupPage::isSkippable() const
{
    return true;
}

QString AccountSetupPage::stepTitle() const
{
    return tr("Join a Server");
}

QString AccountSetupPage::stepSubtitle() const
{
    return tr("Optional — you can always do this later from the menu.");
}

void AccountSetupPage::retranslateUi()
{
    bodyLabel->setText(tr("Playing online needs a server account."));
    registerButton->setText(tr("Register a new account…"));
    connectButton->setText(tr("I already have one — Connect…"));
    skipHintLabel->setText(tr("Just want to play locally? Skip this and connect whenever you're ready."));
}