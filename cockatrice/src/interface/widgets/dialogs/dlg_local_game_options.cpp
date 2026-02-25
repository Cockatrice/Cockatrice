#include "dlg_local_game_options.h"

#include "../../../client/settings/cache_settings.h"
#include "game_zone_options_widget.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

DlgLocalGameOptions::DlgLocalGameOptions(QWidget *parent) : QDialog(parent)
{
    // General settings group
    numberPlayersLabel = new QLabel(tr("P&layers:"));
    numberPlayersEdit = new QSpinBox();
    numberPlayersEdit->setMinimum(1);
    numberPlayersEdit->setMaximum(8);
    numberPlayersEdit->setValue(1);
    numberPlayersLabel->setBuddy(numberPlayersEdit);

    auto *generalGrid = new QGridLayout;
    generalGrid->addWidget(numberPlayersLabel, 0, 0);
    generalGrid->addWidget(numberPlayersEdit, 0, 1);
    generalGroupBox = new QGroupBox(tr("General"));
    generalGroupBox->setLayout(generalGrid);

    // Game setup options group
    startingLifeTotalLabel = new QLabel(tr("Starting life total:"));
    startingLifeTotalEdit = new QSpinBox();
    startingLifeTotalEdit->setMinimum(1);
    startingLifeTotalEdit->setMaximum(99999);
    startingLifeTotalEdit->setValue(20);
    startingLifeTotalLabel->setBuddy(startingLifeTotalEdit);

    zoneOptionsWidget = new GameZoneOptionsWidget(this);
    spectatorsSeeEverythingCheckBox = new QCheckBox(tr("Spectators can see &hands"));

    auto *gameSetupGrid = new QGridLayout;
    gameSetupGrid->addWidget(startingLifeTotalLabel, 0, 0);
    gameSetupGrid->addWidget(startingLifeTotalEdit, 0, 1);
    gameSetupGrid->addWidget(zoneOptionsWidget, 1, 0, 1, 2);
    gameSetupGrid->addWidget(spectatorsSeeEverythingCheckBox, 2, 0, 1, 2);
    gameSetupOptionsGroupBox = new QGroupBox(tr("Game setup options"));
    gameSetupOptionsGroupBox->setLayout(gameSetupGrid);

    // Remember settings checkbox
    rememberSettingsCheckBox = new QCheckBox(tr("Re&member settings"));

    // Button box
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgLocalGameOptions::actOK);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgLocalGameOptions::reject);

    // Main layout
    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(generalGroupBox);
    mainLayout->addWidget(gameSetupOptionsGroupBox);
    mainLayout->addWidget(rememberSettingsCheckBox);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    // Load settings
    rememberSettingsCheckBox->setChecked(SettingsCache::instance().getRememberGameSettings());
    if (rememberSettingsCheckBox->isChecked()) {
        numberPlayersEdit->setValue(SettingsCache::instance().getMaxPlayers());
        startingLifeTotalEdit->setValue(SettingsCache::instance().getDefaultStartingLifeTotal());
        zoneOptionsWidget->loadFromSettings();
        spectatorsSeeEverythingCheckBox->setChecked(SettingsCache::instance().getSpectatorsCanSeeEverything());
    } else {
        zoneOptionsWidget->resetToDefaults();
    }

    setWindowTitle(tr("Local game options"));
    setFixedHeight(sizeHint().height());

    numberPlayersEdit->setFocus();
}

void DlgLocalGameOptions::actOK()
{
    // Save settings if remember is checked
    SettingsCache::instance().setRememberGameSettings(rememberSettingsCheckBox->isChecked());
    if (rememberSettingsCheckBox->isChecked()) {
        SettingsCache::instance().setMaxPlayers(numberPlayersEdit->value());
        SettingsCache::instance().setDefaultStartingLifeTotal(startingLifeTotalEdit->value());
        zoneOptionsWidget->saveToSettings();
        SettingsCache::instance().setSpectatorsCanSeeEverything(spectatorsSeeEverythingCheckBox->isChecked());
    }

    accept();
}

int DlgLocalGameOptions::getNumberPlayers() const
{
    return numberPlayersEdit->value();
}

int DlgLocalGameOptions::getStartingLifeTotal() const
{
    return startingLifeTotalEdit->value();
}

bool DlgLocalGameOptions::getEnableCommandZone() const
{
    return zoneOptionsWidget->enableCommandZone();
}

bool DlgLocalGameOptions::getEnableCompanionZone() const
{
    return zoneOptionsWidget->enableCompanionZone();
}

bool DlgLocalGameOptions::getEnableBackgroundZone() const
{
    return zoneOptionsWidget->enableBackgroundZone();
}

bool DlgLocalGameOptions::getSpectatorsSeeEverything() const
{
    return spectatorsSeeEverythingCheckBox->isChecked();
}
