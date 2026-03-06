#include "dlg_local_game_options.h"

#include "../../../client/settings/cache_settings.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

DlgLocalGameOptions::DlgLocalGameOptions(QWidget *parent) : QDialog(parent)
{
    numberPlayersLabel = new QLabel(tr("P&layers:"), this);
    numberPlayersEdit = new QSpinBox(this);
    numberPlayersEdit->setMinimum(1);
    numberPlayersEdit->setMaximum(8);
    numberPlayersEdit->setValue(1);
    numberPlayersLabel->setBuddy(numberPlayersEdit);

    auto *generalGrid = new QGridLayout;
    generalGrid->addWidget(numberPlayersLabel, 0, 0);
    generalGrid->addWidget(numberPlayersEdit, 0, 1);
    generalGroupBox = new QGroupBox(tr("General"), this);
    generalGroupBox->setLayout(generalGrid);

    startingLifeTotalLabel = new QLabel(tr("Starting &life total:"), this);
    startingLifeTotalEdit = new QSpinBox(this);
    startingLifeTotalEdit->setMinimum(1);
    startingLifeTotalEdit->setMaximum(99999);
    startingLifeTotalEdit->setValue(20);
    startingLifeTotalLabel->setBuddy(startingLifeTotalEdit);

    spectatorsSeeEverythingCheckBox = new QCheckBox(tr("Spectators can see &everything"), this);

    auto *gameSetupGrid = new QGridLayout;
    gameSetupGrid->addWidget(startingLifeTotalLabel, 0, 0);
    gameSetupGrid->addWidget(startingLifeTotalEdit, 0, 1);
    gameSetupGrid->addWidget(spectatorsSeeEverythingCheckBox, 1, 0, 1, 2);
    gameSetupOptionsGroupBox = new QGroupBox(tr("Game setup options"), this);
    gameSetupOptionsGroupBox->setLayout(gameSetupGrid);

    rememberSettingsCheckBox = new QCheckBox(tr("Re&member settings"), this);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgLocalGameOptions::actOK);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgLocalGameOptions::reject);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(generalGroupBox);
    mainLayout->addWidget(gameSetupOptionsGroupBox);
    mainLayout->addWidget(rememberSettingsCheckBox);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    rememberSettingsCheckBox->setChecked(SettingsCache::instance().getRememberGameSettings());
    if (rememberSettingsCheckBox->isChecked()) {
        numberPlayersEdit->setValue(SettingsCache::instance().getMaxPlayers());
        startingLifeTotalEdit->setValue(SettingsCache::instance().getDefaultStartingLifeTotal());
        spectatorsSeeEverythingCheckBox->setChecked(SettingsCache::instance().getSpectatorsCanSeeEverything());
    }

    setWindowTitle(tr("Local game options"));
    setFixedHeight(sizeHint().height());

    numberPlayersEdit->setFocus();
}

void DlgLocalGameOptions::actOK()
{
    SettingsCache::instance().setRememberGameSettings(rememberSettingsCheckBox->isChecked());
    if (rememberSettingsCheckBox->isChecked()) {
        SettingsCache::instance().setMaxPlayers(numberPlayersEdit->value());
        SettingsCache::instance().setDefaultStartingLifeTotal(startingLifeTotalEdit->value());
        SettingsCache::instance().setSpectatorsCanSeeEverything(spectatorsSeeEverythingCheckBox->isChecked());
    }

    accept();
}

LocalGameOptions DlgLocalGameOptions::getOptions() const
{
    return LocalGameOptions{
        .numberPlayers = numberPlayersEdit->value(),
        .startingLifeTotal = startingLifeTotalEdit->value(),
        .spectatorsSeeEverything = spectatorsSeeEverythingCheckBox->isChecked(),
    };
}
