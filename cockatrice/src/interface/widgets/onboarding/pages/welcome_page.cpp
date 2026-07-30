#include "welcome_page.h"

#include <QLabel>
#include <QVBoxLayout>

WelcomePage::WelcomePage(QWidget *parent) : FirstRunWizardPage(parent)
{
    bodyLabel = new QLabel(this);
    bodyLabel->setWordWrap(true);
    bodyLabel->setAlignment(Qt::AlignCenter);

    auto *layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(bodyLabel);
    layout->addStretch();

    retranslateUi();
}

QString WelcomePage::stepTitle() const
{
    return tr("Welcome!");
}

void WelcomePage::retranslateUi()
{
    bodyLabel->setText(tr("Let's get you set up. This will only take a minute — "
                          "we'll grab the card database, pick a look you like, "
                          "and get you ready to connect to a server.\n\n"
                          "You can change any of this later from Settings."));
}