#include "printing_disabled_info_widget.h"

#include "../dialogs/override_printing_warning.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <qguiapplication.h>

PrintingDisabledInfoWidget::PrintingDisabledInfoWidget(QWidget *parent) : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->setObjectName("PrintingDisabledInfoWidgetFrame");

    QLabel *imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    QIcon icon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
    imageLabel->setPixmap(icon.pixmap({60, 60}));

    textLabel = new QLabel(this);
    textLabel->setWordWrap(true);
    textLabel->setAlignment(Qt::AlignCenter);

    settingsButton = new QPushButton(this);
    connect(settingsButton, &QPushButton::clicked, this, &PrintingDisabledInfoWidget::disableOverridePrintings);

    auto buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(settingsButton);
    buttonLayout->addStretch();

    layout->addStretch();
    layout->addWidget(imageLabel);
    layout->addWidget(textLabel);
    layout->addLayout(buttonLayout);
    layout->addStretch();

    retranslateUi();
}

void PrintingDisabledInfoWidget::retranslateUi()
{
    textLabel->setText(
        tr("The Printing Selector is disabled because you have currently enabled the setting to override all "
           "selected printings with personal set preferences.\n\n"
           "This setting means you'll only see the default printing for each card, instead of being able to select a "
           "printing, and will not see the printings other people have selected.\n\n"));
    settingsButton->setText(tr("Enable printings again"));
}

void PrintingDisabledInfoWidget::disableOverridePrintings()
{
    OverridePrintingWarning::execMessageBox(this, false);
}