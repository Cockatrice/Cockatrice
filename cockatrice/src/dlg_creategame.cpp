#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QSet>
#include "dlg_creategame.h"
#include "tab_room.h"

#include "pending_command.h"
#include "pb/room_commands.pb.h"
#include "pb/serverinfo_game.pb.h"
#include "pb/response.pb.h"

void DlgCreateGame::sharedCtor()
{
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
    
    QVBoxLayout *gameTypeLayout = new QVBoxLayout;
    QMapIterator<int, QString> gameTypeIterator(gameTypes);
    while (gameTypeIterator.hasNext()) {
        gameTypeIterator.next();
        QCheckBox *gameTypeCheckBox = new QCheckBox(gameTypeIterator.value());
        gameTypeLayout->addWidget(gameTypeCheckBox);
        gameTypeCheckBoxes.insert(gameTypeIterator.key(), gameTypeCheckBox);
    }
    QGroupBox *gameTypeGroupBox = new QGroupBox(tr("Game type"));
    gameTypeGroupBox->setLayout(gameTypeLayout);
    
    passwordLabel = new QLabel(tr("&Password:"));
    passwordEdit = new QLineEdit;
    passwordLabel->setBuddy(passwordEdit);

    onlyBuddiesCheckBox = new QCheckBox(tr("Only &buddies can join"));
    onlyRegisteredCheckBox = new QCheckBox(tr("Only &registered users can join"));
    if (room && room->getUserInfo()->user_level() & ServerInfo_User::IsRegistered)
        onlyRegisteredCheckBox->setChecked(true);
    
    QGridLayout *joinRestrictionsLayout = new QGridLayout;
    joinRestrictionsLayout->addWidget(passwordLabel, 0, 0);
    joinRestrictionsLayout->addWidget(passwordEdit, 0, 1);
    joinRestrictionsLayout->addWidget(onlyBuddiesCheckBox, 1, 0, 1, 2);
    joinRestrictionsLayout->addWidget(onlyRegisteredCheckBox, 2, 0, 1, 2);
    
    QGroupBox *joinRestrictionsGroupBox = new QGroupBox(tr("Joining restrictions"));
    joinRestrictionsGroupBox->setLayout(joinRestrictionsLayout);
    
    spectatorsAllowedCheckBox = new QCheckBox(tr("&Spectators allowed"));
    spectatorsAllowedCheckBox->setChecked(true);
    connect(spectatorsAllowedCheckBox, SIGNAL(stateChanged(int)), this, SLOT(spectatorsAllowedChanged(int)));
    spectatorsNeedPasswordCheckBox = new QCheckBox(tr("Spectators &need a password to join"));
    spectatorsCanTalkCheckBox = new QCheckBox(tr("Spectators can &chat"));
    spectatorsSeeEverythingCheckBox = new QCheckBox(tr("Spectators see &everything"));
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
    : QDialog(parent), room(_room), gameTypes(_gameTypes)
{
    sharedCtor();
    
    buttonBox->addButton(QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOK()));
    
    setWindowTitle(tr("Create game"));
}

DlgCreateGame::DlgCreateGame(const ServerInfo_Game &gameInfo, const QMap<int, QString> &_gameTypes, QWidget *parent)
    : QDialog(parent), room(0), gameTypes(_gameTypes)
{
    sharedCtor();
    
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
        
        QCheckBox *gameTypeCheckBox = gameTypeCheckBoxes.value(gameTypeIterator.key());
        gameTypeCheckBox->setEnabled(false);
        gameTypeCheckBox->setChecked(types.contains(gameTypeIterator.key()));
    }
    
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    
    setWindowTitle(tr("Game information"));
}

void DlgCreateGame::actOK()
{
    Command_CreateGame cmd;
    cmd.set_description(descriptionEdit->text().toStdString());
    cmd.set_password(passwordEdit->text().toStdString());
    cmd.set_max_players(maxPlayersEdit->value());
    cmd.set_only_buddies(onlyBuddiesCheckBox->isChecked());
    cmd.set_only_registered(onlyRegisteredCheckBox->isChecked());
    cmd.set_spectators_allowed(spectatorsAllowedCheckBox->isChecked());
    cmd.set_spectators_need_password(spectatorsNeedPasswordCheckBox->isChecked());
    cmd.set_spectators_can_talk(spectatorsCanTalkCheckBox->isChecked());
    cmd.set_spectators_see_everything(spectatorsSeeEverythingCheckBox->isChecked());
    
    QMapIterator<int, QCheckBox *> gameTypeCheckBoxIterator(gameTypeCheckBoxes);
    while (gameTypeCheckBoxIterator.hasNext()) {
        gameTypeCheckBoxIterator.next();
        if (gameTypeCheckBoxIterator.value()->isChecked())
            cmd.add_game_type_ids(gameTypeCheckBoxIterator.key());
    }
    
    PendingCommand *pend = room->prepareRoomCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(checkResponse(Response)));
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

void DlgCreateGame::spectatorsAllowedChanged(int state)
{
    spectatorsNeedPasswordCheckBox->setEnabled(state);
    spectatorsCanTalkCheckBox->setEnabled(state);
    spectatorsSeeEverythingCheckBox->setEnabled(state);
}
