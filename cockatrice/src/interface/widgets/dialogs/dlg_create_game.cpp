#include "dlg_create_game.h"

#include "../../../client/settings/cache_settings.h"
#include "../interface/widgets/tabs/tab_room.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSet>
#include <QSpinBox>
#include <QWizard>
#include <libcockatrice/protocol/pb/serverinfo_game.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/utility/trice_limits.h>

void DlgCreateGame::sharedCtor()
{
    rememberGameSettings = new QCheckBox(tr("Re&member settings"));
    descriptionLabel = new QLabel(tr("&Description:"));
    descriptionEdit = new QLineEdit;
    descriptionEdit->setMaxLength(MAX_NAME_LENGTH);
    descriptionLabel->setBuddy(descriptionEdit);

    maxPlayersLabel = new QLabel(tr("P&layers:"));
    maxPlayersEdit = new QSpinBox();
    maxPlayersEdit->setMinimum(1);
    maxPlayersEdit->setMaximum(100);
    maxPlayersEdit->setValue(2);
    maxPlayersLabel->setBuddy(maxPlayersEdit);

    auto *generalGrid = new QGridLayout;
    generalGrid->addWidget(descriptionLabel, 0, 0);
    generalGrid->addWidget(descriptionEdit, 0, 1);
    generalGrid->addWidget(maxPlayersLabel, 1, 0);
    generalGrid->addWidget(maxPlayersEdit, 1, 1);
    generalGroupBox = new QGroupBox(tr("General"));
    generalGroupBox->setLayout(generalGrid);

    auto *gameTypeLayout = new QVBoxLayout;
    QMapIterator<int, QString> gameTypeIterator(gameTypes);
    while (gameTypeIterator.hasNext()) {
        gameTypeIterator.next();
        auto *gameTypeRadioButton = new QRadioButton(gameTypeIterator.value(), this);
        gameTypeLayout->addWidget(gameTypeRadioButton);
        gameTypeCheckBoxes.insert(gameTypeIterator.key(), gameTypeRadioButton);
        bool isChecked = SettingsCache::instance().getGameTypes().contains(gameTypeIterator.value() + ", ");
        gameTypeCheckBoxes[gameTypeIterator.key()]->setChecked(isChecked);
    }
    auto *gameTypeGroupBox = new QGroupBox(tr("Game type"));
    gameTypeGroupBox->setLayout(gameTypeLayout);

    passwordLabel = new QLabel(tr("&Password:"));
    passwordEdit = new QLineEdit;
    passwordEdit->setMaxLength(MAX_NAME_LENGTH);
    passwordLabel->setBuddy(passwordEdit);

    onlyBuddiesCheckBox = new QCheckBox(tr("Only &buddies can join"));
    onlyRegisteredCheckBox = new QCheckBox(tr("Only &registered users can join"));
    if (room && room->getUserInfo()->user_level() & ServerInfo_User::IsRegistered) {
        onlyRegisteredCheckBox->setChecked(true);
    } else {
        onlyBuddiesCheckBox->setEnabled(false);
        onlyRegisteredCheckBox->setEnabled(false);
    }

    auto *joinRestrictionsLayout = new QGridLayout;
    joinRestrictionsLayout->addWidget(passwordLabel, 0, 0);
    joinRestrictionsLayout->addWidget(passwordEdit, 0, 1);
    joinRestrictionsLayout->addWidget(onlyBuddiesCheckBox, 1, 0, 1, 2);
    joinRestrictionsLayout->addWidget(onlyRegisteredCheckBox, 2, 0, 1, 2);

    auto *joinRestrictionsGroupBox = new QGroupBox(tr("Joining restrictions"));
    joinRestrictionsGroupBox->setLayout(joinRestrictionsLayout);

    spectatorsAllowedCheckBox = new QCheckBox(tr("&Spectators can watch"));
    spectatorsAllowedCheckBox->setChecked(true);
    connect(spectatorsAllowedCheckBox, &QCheckBox::QT_STATE_CHANGED, this, &DlgCreateGame::spectatorsAllowedChanged);
    spectatorsNeedPasswordCheckBox = new QCheckBox(tr("Spectators &need a password to watch"));
    spectatorsCanTalkCheckBox = new QCheckBox(tr("Spectators can &chat"));
    spectatorsSeeEverythingCheckBox = new QCheckBox(tr("Spectators can see &hands"));
    createGameAsSpectatorCheckBox = new QCheckBox(tr("Create game as spectator"));
    auto *spectatorsLayout = new QVBoxLayout;
    spectatorsLayout->addWidget(spectatorsAllowedCheckBox);
    spectatorsLayout->addWidget(spectatorsNeedPasswordCheckBox);
    spectatorsLayout->addWidget(spectatorsCanTalkCheckBox);
    spectatorsLayout->addWidget(spectatorsSeeEverythingCheckBox);
    spectatorsLayout->addWidget(createGameAsSpectatorCheckBox);
    spectatorsGroupBox = new QGroupBox(tr("Spectators"));
    spectatorsGroupBox->setLayout(spectatorsLayout);

    startingLifeTotalLabel = new QLabel(tr("Starting life total:"));
    startingLifeTotalEdit = new QSpinBox();
    startingLifeTotalEdit->setMinimum(1);
    startingLifeTotalEdit->setMaximum(99999); ///< Arbitrary but we can raise this when people start complaining.
    startingLifeTotalEdit->setValue(20);
    startingLifeTotalLabel->setBuddy(startingLifeTotalEdit);

    shareDecklistsOnLoadLabel = new QLabel(tr("Open decklists in lobby"));
    shareDecklistsOnLoadCheckBox = new QCheckBox();
    shareDecklistsOnLoadLabel->setBuddy(shareDecklistsOnLoadCheckBox);

    auto *gameSetupOptionsLayout = new QGridLayout;
    gameSetupOptionsLayout->addWidget(startingLifeTotalLabel, 0, 0);
    gameSetupOptionsLayout->addWidget(startingLifeTotalEdit, 0, 1);
    gameSetupOptionsLayout->addWidget(shareDecklistsOnLoadLabel, 1, 0);
    gameSetupOptionsLayout->addWidget(shareDecklistsOnLoadCheckBox, 1, 1);
    gameSetupOptionsGroupBox = new QGroupBox(tr("Game setup options"));
    gameSetupOptionsGroupBox->setLayout(gameSetupOptionsLayout);

    auto *grid = new QGridLayout;

    // Top row
    grid->addWidget(generalGroupBox, 0, 0);
    grid->addWidget(joinRestrictionsGroupBox, 0, 1);

    // Middle row: left column
    grid->addWidget(gameTypeGroupBox, 1, 0);

    // Middle row: right column (game setup + spectators)
    auto *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(spectatorsGroupBox, Qt::AlignTop); // top
    rightLayout->addWidget(gameSetupOptionsGroupBox);         // bottom

    grid->addLayout(rightLayout, 1, 1);

    // Bottom row
    grid->addWidget(rememberGameSettings, 3, 0, 1, 2); // span both columns if needed

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgCreateGame::reject);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    setFixedHeight(sizeHint().height());
}

DlgCreateGame::DlgCreateGame(TabRoom *_room, const QMap<int, QString> &_gameTypes, QWidget *parent)
    : QDialog(parent), room(_room), gameTypes(_gameTypes)
{
    sharedCtor();

    rememberGameSettings->setChecked(SettingsCache::instance().getRememberGameSettings());
    descriptionEdit->setText(SettingsCache::instance().getGameDescription());
    maxPlayersEdit->setValue(SettingsCache::instance().getMaxPlayers());
    if (room && room->getUserInfo()->user_level() & ServerInfo_User::IsRegistered) {
        onlyBuddiesCheckBox->setChecked(SettingsCache::instance().getOnlyBuddies());
        onlyRegisteredCheckBox->setChecked(SettingsCache::instance().getOnlyRegistered());
    } else {
        onlyBuddiesCheckBox->setEnabled(false);
        onlyRegisteredCheckBox->setEnabled(false);
    }
    spectatorsAllowedCheckBox->setChecked(SettingsCache::instance().getSpectatorsAllowed());
    spectatorsNeedPasswordCheckBox->setChecked(SettingsCache::instance().getSpectatorsNeedPassword());
    spectatorsCanTalkCheckBox->setChecked(SettingsCache::instance().getSpectatorsCanTalk());
    spectatorsSeeEverythingCheckBox->setChecked(SettingsCache::instance().getSpectatorsCanSeeEverything());
    createGameAsSpectatorCheckBox->setChecked(SettingsCache::instance().getCreateGameAsSpectator());
    startingLifeTotalEdit->setValue(SettingsCache::instance().getDefaultStartingLifeTotal());
    shareDecklistsOnLoadCheckBox->setChecked(SettingsCache::instance().getShareDecklistsOnLoad());

    if (!rememberGameSettings->isChecked()) {
        actReset();
    }

    descriptionEdit->setFocus();
    clearButton = new QPushButton(tr("&Clear"));
    buttonBox->addButton(QDialogButtonBox::Cancel);
    buttonBox->addButton(clearButton, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgCreateGame::actOK);
    connect(clearButton, &QPushButton::clicked, this, &DlgCreateGame::actReset);

    setWindowTitle(tr("Create game"));
}

DlgCreateGame::DlgCreateGame(const ServerInfo_Game &gameInfo, const QMap<int, QString> &_gameTypes, QWidget *parent)
    : QDialog(parent), room(0), gameTypes(_gameTypes)
{
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
    createGameAsSpectatorCheckBox->setEnabled(false);
    startingLifeTotalEdit->setEnabled(false);
    shareDecklistsOnLoadCheckBox->setEnabled(false);

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

    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgCreateGame::accept);

    setWindowTitle(tr("Game information"));
}

void DlgCreateGame::actReset()
{
    descriptionEdit->setText("");
    maxPlayersEdit->setValue(2);

    passwordEdit->setText("");
    onlyBuddiesCheckBox->setChecked(false);
    onlyRegisteredCheckBox->setChecked(room && room->getUserInfo()->user_level() & ServerInfo_User::IsRegistered);

    spectatorsAllowedCheckBox->setChecked(true);
    spectatorsNeedPasswordCheckBox->setChecked(false);
    spectatorsCanTalkCheckBox->setChecked(false);
    spectatorsSeeEverythingCheckBox->setChecked(false);
    createGameAsSpectatorCheckBox->setChecked(false);

    startingLifeTotalEdit->setValue(20);
    shareDecklistsOnLoadCheckBox->setChecked(false);

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

void DlgCreateGame::actOK()
{
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
    cmd.set_join_as_judge(QApplication::keyboardModifiers() & Qt::ShiftModifier);
    cmd.set_join_as_spectator(createGameAsSpectatorCheckBox->isChecked());
    cmd.set_starting_life_total(startingLifeTotalEdit->value());
    cmd.set_share_decklists_on_load(shareDecklistsOnLoadCheckBox->isChecked());

    auto _gameTypes = QString();
    QMapIterator<int, QRadioButton *> gameTypeCheckBoxIterator(gameTypeCheckBoxes);
    while (gameTypeCheckBoxIterator.hasNext()) {
        gameTypeCheckBoxIterator.next();
        if (gameTypeCheckBoxIterator.value()->isChecked()) {
            cmd.add_game_type_ids(gameTypeCheckBoxIterator.key());
            _gameTypes += gameTypeCheckBoxIterator.value()->text() + ", ";
        }
    }

    SettingsCache::instance().setRememberGameSettings(rememberGameSettings->isChecked());
    if (rememberGameSettings->isChecked()) {
        SettingsCache::instance().setGameDescription(descriptionEdit->text());
        SettingsCache::instance().setMaxPlayers(maxPlayersEdit->value());
        SettingsCache::instance().setOnlyBuddies(onlyBuddiesCheckBox->isChecked());
        SettingsCache::instance().setOnlyRegistered(onlyRegisteredCheckBox->isChecked());
        SettingsCache::instance().setSpectatorsAllowed(spectatorsAllowedCheckBox->isChecked());
        SettingsCache::instance().setSpectatorsNeedPassword(spectatorsNeedPasswordCheckBox->isChecked());
        SettingsCache::instance().setSpectatorsCanTalk(spectatorsCanTalkCheckBox->isChecked());
        SettingsCache::instance().setSpectatorsCanSeeEverything(spectatorsSeeEverythingCheckBox->isChecked());
        SettingsCache::instance().setCreateGameAsSpectator(createGameAsSpectatorCheckBox->isChecked());
        SettingsCache::instance().setDefaultStartingLifeTotal(startingLifeTotalEdit->value());
        SettingsCache::instance().setShareDecklistsOnLoad(shareDecklistsOnLoadCheckBox->isChecked());
        SettingsCache::instance().setGameTypes(_gameTypes);
    }
    PendingCommand *pend = room->prepareRoomCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &DlgCreateGame::checkResponse);
    room->sendRoomCommand(pend);

    buttonBox->setEnabled(false);
}

void DlgCreateGame::checkResponse(const Response &response)
{
    buttonBox->setEnabled(true);

    if (response.response_code() == Response::RespOk)
        accept();
    else {
        QMessageBox::critical(this, tr("Error"), tr("Server error."));
        return;
    }
}

void DlgCreateGame::spectatorsAllowedChanged(QT_STATE_CHANGED_T state)
{
    spectatorsNeedPasswordCheckBox->setEnabled(state);
    spectatorsCanTalkCheckBox->setEnabled(state);
    spectatorsSeeEverythingCheckBox->setEnabled(state);
}
