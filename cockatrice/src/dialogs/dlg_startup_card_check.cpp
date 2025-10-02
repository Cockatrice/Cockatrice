#include "dlg_startup_card_check.h"

#include <QDate>
#include <libcockatrice/settings/cache_settings.h>

DlgStartupCardCheck::DlgStartupCardCheck(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Card Update Check"));

    layout = new QVBoxLayout(this);

    QDate lastCheckDate = SettingsCache::instance().getLastCardUpdateCheck();
    int daysAgo = lastCheckDate.daysTo(QDate::currentDate());

    instructionLabel = new QLabel(
        tr("It has been more than %2 days since you last checked your card database for updates.\nChoose how you would "
           "like to run the card database updater.\nYou can always change this behavior in the 'General' settings tab.")
            .arg(daysAgo));

    layout->addWidget(instructionLabel);

    group = new QButtonGroup(this);

    foregroundBtn = new QRadioButton(tr("Run in foreground"));
    backgroundBtn = new QRadioButton(tr("Run in background"));
    backgroundAlwaysBtn = new QRadioButton(tr("Run in background and always from now on"));
    dontPromptBtn = new QRadioButton(tr("Don't prompt again and don't run"));
    dontRunBtn = new QRadioButton(tr("Don't run this time"));

    group->addButton(foregroundBtn, 0);
    group->addButton(backgroundBtn, 1);
    group->addButton(backgroundAlwaysBtn, 2);
    group->addButton(dontPromptBtn, 3);
    group->addButton(dontRunBtn, 4);

    foregroundBtn->setChecked(true); // default

    layout->addWidget(foregroundBtn);
    layout->addWidget(backgroundBtn);
    layout->addWidget(backgroundAlwaysBtn);
    layout->addWidget(dontPromptBtn);
    layout->addWidget(dontRunBtn);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
