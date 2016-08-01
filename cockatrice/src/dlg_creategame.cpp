#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>
#include <QRadioButton>
#include <QSpinBox>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QSet>
#include <QWizard>
#include "dlg_creategame.h"
#include "tab_room.h"
#include "settingscache.h"

#include "pending_command.h"
#include "pb/serverinfo_game.pb.h"

void DlgCreateGame::sharedCtor() {
    rememberGameSettings = new QCheckBox(tr("Re&member settings"));
    descriptionLabel = new QLabel(tr("&Description:"));
    descriptionEdit = new QLineEdit;
    descriptionLabel->setBuddy(descriptionEdit);
    descriptionEdit->setMaxLength(60);

    maxPlayersLabel = new QLabel(tr("P&layers:"));
    maxPlayersEdit = new QSpinBox();
    maxPlayersEdit->setMinimum(1);
    maxPlayersEdit->setMaximum(100);
    maxPlayersEdit->setValue(2);
    maxPlayersLabel->setBuddy(maxPlayersEdit);

    QGridLayout *generalGrid = new QGridLayout;
    generalGrid->addWidget(descriptionLabel, 0, 0);
    generalGrid->addWidget(descriptionEdit, 0, 1);
    generalGrid->addWidget(maxPlayersLabel, 1, 0);
    generalGrid->addWidget(maxPlayersEdit, 1, 1);
    generalGrid->addWidget(rememberGameSettings, 2, 0);

    QVBoxLayout *gameTypeLayout = new QVBoxLayout;
    QMapIterator<int, QString> gameTypeIterator(gameTypes);
    while (gameTypeIterator.hasNext()) {
        gameTypeIterator.next();
        QRadioButton *gameTypeRadioButton = new QRadioButton(gameTypeIterator.value(), this);
        gameTypeLayout->addWidget(gameTypeRadioButton);
        gameTypeCheckBoxes.insert(gameTypeIterator.key(), gameTypeRadioButton);
        bool isChecked = settingsCache->getGameTypes().contains(gameTypeIterator.value() + ", ");
        gameTypeCheckBoxes[gameTypeIterator.key()]->setChecked(isChecked);
    }
    QGroupBox *gameTypeGroupBox = new QGroupBox(tr("Game type"));
    gameTypeGroupBox->setLayout(gameTypeLayout);

    passwordLabel = new QLabel(tr("&Password:"));
    passwordEdit = new QLineEdit;
    passwordLabel->setBuddy(passwordEdit);

    onlyBuddiesCheckBox = new QCheckBox(tr("Only &buddies can join"));
    onlyRegisteredCheckBox = new QCheckBox(tr("Only &registered users can join"));
    if (room && room->getUserInfo()->user_level() & ServerInfo_User::IsRegistered) {
        onlyRegisteredCheckBox->setChecked(true);
    } else {
        onlyBuddiesCheckBox->setEnabled(false);
        onlyRegisteredCheckBox->setEnabled(false);
    }

    QGridLayout *joinRestrictionsLayout = new QGridLayout;
    joinRestrictionsLayout->addWidget(passwordLabel, 0, 0);
    joinRestrictionsLayout->addWidget(passwordEdit, 0, 1);
    joinRestrictionsLayout->addWidget(onlyBuddiesCheckBox, 1, 0, 1, 2);
    joinRestrictionsLayout->addWidget(onlyRegisteredCheckBox, 2, 0, 1, 2);

    QGroupBox *joinRestrictionsGroupBox = new QGroupBox(tr("Joining restrictions"));
    joinRestrictionsGroupBox->setLayout(joinRestrictionsLayout);

    spectatorsAllowedCheckBox = new QCheckBox(tr("&Spectators can watch"));
    spectatorsAllowedCheckBox->setChecked(true);
    connect(spectatorsAllowedCheckBox, SIGNAL(stateChanged(int)), this, SLOT(spectatorsAllowedChanged(int)));
    spectatorsNeedPasswordCheckBox = new QCheckBox(tr("Spectators &need a password to watch"));
    spectatorsCanTalkCheckBox = new QCheckBox(tr("Spectators can &chat"));
    spectatorsSeeEverythingCheckBox = new QCheckBox(tr("Spectators can see &hands"));
    QVBoxLayout *spectatorsLayout = new QVBoxLayout;
    spectatorsLayout->addWidget(spectatorsAllowedCheckBox);
    spectatorsLayout->addWidget(spectatorsNeedPasswordCheckBox);
    spectatorsLayout->addWidget(spectatorsCanTalkCheckBox);
    spectatorsLayout->addWidget(spectatorsSeeEverythingCheckBox);
    spectatorsGroupBox = new QGroupBox(tr("Spectators"));
    spectatorsGroupBox->setLayout(spectatorsLayout);

    QGridLayout *grid = new QGridLayout;
    grid->addLayout(generalGrid, 0, 0);
    grid->addWidget(spectatorsGroupBox, 1, 0);
    grid->addWidget(joinRestrictionsGroupBox, 0, 1);
    grid->addWidget(gameTypeGroupBox, 1, 1);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    setFixedHeight(sizeHint().height());
}

DlgCreateGame::DlgCreateGame(TabRoom *_room, const QMap<int, QString> &_gameTypes, QWidget *parent)
        : QDialog(parent), room(_room), gameTypes(_gameTypes) {
    sharedCtor();

    rememberGameSettings->setChecked(settingsCache->getRememberGameSettings());
    descriptionEdit->setText(settingsCache->getGameDescription());
    maxPlayersEdit->setValue(settingsCache->getMaxPlayers());
    if (room && room->getUserInfo()->user_level() & ServerInfo_User::IsRegistered) {
        onlyBuddiesCheckBox->setChecked(settingsCache->getOnlyBuddies());
        onlyRegisteredCheckBox->setChecked(settingsCache->getOnlyRegistered());
    } else {
        onlyBuddiesCheckBox->setEnabled(false);
        onlyRegisteredCheckBox->setEnabled(false);
    }
    spectatorsAllowedCheckBox->setChecked(settingsCache->getSpectatorsAllowed());
    spectatorsNeedPasswordCheckBox->setChecked(settingsCache->getSpectatorsNeedPassword());
    spectatorsCanTalkCheckBox->setChecked(settingsCache->getSpectatorsCanTalk());
    spectatorsSeeEverythingCheckBox->setChecked(settingsCache->getSpectatorsCanSeeEverything());

    if (!rememberGameSettings->isChecked()) {
        actReset();
    }

    descriptionEdit->setFocus();
    clearButton = new QPushButton(tr("&Clear"));
    buttonBox->addButton(QDialogButtonBox::Cancel);
    buttonBox->addButton(clearButton, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOK()));
    connect(clearButton, SIGNAL(clicked()), this, SLOT(actReset()));

    setWindowTitle(tr("Create game"));
}

DlgCreateGame::DlgCreateGame(const ServerInfo_Game &gameInfo, const QMap<int, QString> &_gameTypes, QWidget *parent)
        : QDialog(parent), room(0), gameTypes(_gameTypes) {
    sharedCtor();

    rememberGameSettings->setEnabled(false);
    descriptionEdit->setEnabled(false);
    maxPlayersEdit->setEnabled(false);
    passwordEdit->setEnabled(false);
    onlyBuddiesCheckBox->setEnabled(false);
    onlyRegisteredCheckBox->setEnabled(false);
    spectatorsAllowedCheckBox->setEnabled(false);
    spectatorsNeedPasswordCheckBox->setEnabled(false);
    spectatorsCanTalkCheckBox->setEnabled(false);
    spectatorsSeeEverythingCheckBox->setEnabled(false);

    descriptionEdit->setText(QString::fromStdString(gameInfo.description()));
    maxPlayersEdit->setValue(gameInfo.max_players());
    onlyBuddiesCheckBox->setChecked(gameInfo.only_buddies());
    onlyRegisteredCheckBox->setChecked(gameInfo.only_registered());
    spectatorsAllowedCheckBox->setChecked(gameInfo.spectators_allowed());
    spectatorsNeedPasswordCheckBox->setChecked(gameInfo.spectators_need_password());
    spectatorsCanTalkCheckBox->setChecked(gameInfo.spectators_can_chat());
    spectatorsSeeEverythingCheckBox->setChecked(gameInfo.spectators_omniscient());

    QSet<int> types;
    for (int i = 0; i < gameInfo.game_types_size(); ++i)
        types.insert(gameInfo.game_types(i));

    QMapIterator<int, QString> gameTypeIterator(gameTypes);
    while (gameTypeIterator.hasNext()) {
        gameTypeIterator.next();

        QRadioButton *gameTypeCheckBox = gameTypeCheckBoxes.value(gameTypeIterator.key());
        gameTypeCheckBox->setEnabled(false);
        gameTypeCheckBox->setChecked(types.contains(gameTypeIterator.key()));
    }

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    setWindowTitle(tr("Game information"));
}

void DlgCreateGame::actReset() {
    descriptionEdit->setText("");
    maxPlayersEdit->setValue(2);

    passwordEdit->setText("");
    onlyBuddiesCheckBox->setChecked(false);
    onlyRegisteredCheckBox->setChecked(room && room->getUserInfo()->user_level() & ServerInfo_User::IsRegistered);

    spectatorsAllowedCheckBox->setChecked(true);
    spectatorsNeedPasswordCheckBox->setChecked(false);
    spectatorsCanTalkCheckBox->setChecked(false);
    spectatorsSeeEverythingCheckBox->setChecked(false);

    QMapIterator<int, QRadioButton *> gameTypeCheckBoxIterator(gameTypeCheckBoxes);
    while (gameTypeCheckBoxIterator.hasNext()) {
        gameTypeCheckBoxIterator.next();
        // must set auto enclusive to false to be able to set the check to false
        gameTypeCheckBoxIterator.value()->setAutoExclusive(false);
        gameTypeCheckBoxIterator.value()->setChecked(false);
        gameTypeCheckBoxIterator.value()->setAutoExclusive(true);
    }

    descriptionEdit->setFocus();
}


void DlgCreateGame::actOK() {
    Command_CreateGame cmd;
    cmd.set_description(descriptionEdit->text().simplified().toStdString());
    cmd.set_password(passwordEdit->text().toStdString());
    cmd.set_max_players(maxPlayersEdit->value());
    cmd.set_only_buddies(onlyBuddiesCheckBox->isChecked());
    cmd.set_only_registered(onlyRegisteredCheckBox->isChecked());
    cmd.set_spectators_allowed(spectatorsAllowedCheckBox->isChecked());
    cmd.set_spectators_need_password(spectatorsNeedPasswordCheckBox->isChecked());
    cmd.set_spectators_can_talk(spectatorsCanTalkCheckBox->isChecked());
    cmd.set_spectators_see_everything(spectatorsSeeEverythingCheckBox->isChecked());

    QString gameTypes = QString();
    QMapIterator<int, QRadioButton *> gameTypeCheckBoxIterator(gameTypeCheckBoxes);
    while (gameTypeCheckBoxIterator.hasNext()) {
        gameTypeCheckBoxIterator.next();
        if (gameTypeCheckBoxIterator.value()->isChecked()) {
            cmd.add_game_type_ids(gameTypeCheckBoxIterator.key());
            gameTypes += gameTypeCheckBoxIterator.value()->text() + ", ";
        }
    }

    settingsCache->setRememberGameSettings(rememberGameSettings->isChecked());
    if (rememberGameSettings->isChecked()) {
        settingsCache->setGameDescription(descriptionEdit->text());
        settingsCache->setMaxPlayers(maxPlayersEdit->value());
        settingsCache->setOnlyBuddies(onlyBuddiesCheckBox->isChecked());
        settingsCache->setOnlyRegistered(onlyRegisteredCheckBox->isChecked());
        settingsCache->setSpectatorsAllowed(spectatorsAllowedCheckBox->isChecked());
        settingsCache->setSpectatorsNeedPassword(spectatorsNeedPasswordCheckBox->isChecked());
        settingsCache->setSpectatorsCanTalk(spectatorsCanTalkCheckBox->isChecked());
        settingsCache->setSpectatorsCanSeeEverything(spectatorsSeeEverythingCheckBox->isChecked());
        settingsCache->setGameTypes(gameTypes);
    }
    PendingCommand *pend = room->prepareRoomCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(checkResponse(Response)));
    room->sendRoomCommand(pend);

    buttonBox->setEnabled(false);
}

void DlgCreateGame::checkResponse(const Response &response) {
    buttonBox->setEnabled(true);

    if (response.response_code() == Response::RespOk)
        accept();
    else {
        QMessageBox::critical(this, tr("Error"), tr("Server error."));
        return;
    }
}

void DlgCreateGame::spectatorsAllowedChanged(int state) {
    spectatorsNeedPasswordCheckBox->setEnabled(state);
    spectatorsCanTalkCheckBox->setEnabled(state);
    spectatorsSeeEverythingCheckBox->setEnabled(state);
}

