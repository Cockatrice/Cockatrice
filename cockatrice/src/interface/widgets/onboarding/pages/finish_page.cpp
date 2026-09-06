#include "finish_page.h"

#include <QLabel>
#include <QVBoxLayout>

FinishPage::FinishPage(QWidget *parent) : FirstRunWizardPage(parent)
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

QString FinishPage::stepTitle() const
{
    return tr("You're All Set");
}

void FinishPage::retranslateUi()
{
    bodyLabel->setText(
        tr("That's everything for now. Jump into Settings any time to change your mind about any of this.\n\n"
           "Have fun!"));
}