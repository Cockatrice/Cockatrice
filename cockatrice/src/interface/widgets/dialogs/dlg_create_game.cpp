#include "dlg_create_game.h"

#include "../../../client/settings/cache_settings.h"
#include "../game_hosting/create_game_form_widget.h"
#include "../game_hosting/game_creation_options.h"
#include "../tabs/tab_room.h"
#include "libcockatrice/protocol/pb/response.pb.h"
#include "libcockatrice/protocol/pb/room_commands.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_game.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_user.pb.h"
#include "libcockatrice/protocol/pending_command.h"
#include "libcockatrice/settings/game_settings.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

DlgCreateGame::DlgCreateGame(TabRoom *_room, const QMap<int, QString> &_gameTypes, QWidget *parent)
    : QDialog(parent), room(_room)
{
    const bool registered = room && (room->getUserInfo()->user_level() & ServerInfo_User::IsRegistered);
    const bool judge = room && (room->getUserInfo()->user_level() & ServerInfo_User::IsJudge);

    form = new CreateGameFormWidget(_gameTypes, registered, judge, this);
    form->setRememberSettingsChecked(SettingsCache::instance().game().getRememberGameSettings());
    form->setOptions(GameCreationOptions::fromSettings(SettingsCache::instance().game(), _gameTypes, registered));
    if (!form->isRememberSettingsChecked()) {
        form->resetToDefaults();
    }
    form->focusDescription();

    errorLabel = new QLabel(this);
    errorLabel->setObjectName("createGameErrorLabel");
    errorLabel->setWordWrap(true);
    errorLabel->setVisible(false);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgCreateGame::reject);
    clearButton = new QPushButton(tr("&Clear"), this);
    buttonBox->addButton(QDialogButtonBox::Cancel);
    buttonBox->addButton(clearButton, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgCreateGame::actOK);
    connect(clearButton, &QPushButton::clicked, this, &DlgCreateGame::actReset);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(form);
    mainLayout->addWidget(errorLabel);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("Create game"));
}

DlgCreateGame::DlgCreateGame(const ServerInfo_Game &gameInfo, const QMap<int, QString> &_gameTypes, QWidget *parent)
    : QDialog(parent), room(nullptr)
{
    form = new CreateGameFormWidget(_gameTypes, false, false, this);
    form->setOptions(GameCreationOptions::fromGameInfo(gameInfo, _gameTypes));
    form->setReadOnly(true);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgCreateGame::accept);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(form);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("Game information"));
}

void DlgCreateGame::actReset()
{
    form->resetToDefaults();
    clearError();
}

void DlgCreateGame::actOK()
{
    clearError();

    const GameCreationOptions options = form->options();
    Command_CreateGame cmd = options.toCreateGameCommand();

    const bool rememberSettings = form->isRememberSettingsChecked();
    auto &gameSettings = SettingsCache::instance().game();
    gameSettings.setRememberGameSettings(rememberSettings);
    if (rememberSettings) {
        gameSettings.setGameDescription(options.description);
        gameSettings.setMaxPlayers(options.maxPlayers);
        gameSettings.setOnlyBuddies(options.onlyBuddies);
        gameSettings.setOnlyRegistered(options.onlyRegistered);
        gameSettings.setSpectatorsAllowed(options.spectatorsAllowed);
        gameSettings.setSpectatorsNeedPassword(options.spectatorsNeedPassword);
        gameSettings.setSpectatorsCanTalk(options.spectatorsCanTalk);
        gameSettings.setSpectatorsCanSeeEverything(options.spectatorsSeeEverything);
        gameSettings.setCreateGameAsSpectator(options.joinAsSpectator);
        gameSettings.setDefaultStartingLifeTotal(options.startingLifeTotal);
        gameSettings.setShareDecklistsOnLoad(options.shareDecklistsOnLoad);
        gameSettings.setShufflePlayers(options.shufflePlayers);
        gameSettings.setGameTypes(form->selectedGameTypesSettingString());
    }

    PendingCommand *pend = room->prepareRoomCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &DlgCreateGame::checkResponse);
    room->sendRoomCommand(pend);

    buttonBox->setEnabled(false);
}

void DlgCreateGame::checkResponse(const Response &response)
{
    buttonBox->setEnabled(true);

    if (response.response_code() == Response::RespOk) {
        accept();
    } else {
        showError(tr("Server error."));
    }
}

void DlgCreateGame::showError(const QString &message)
{
    if (!errorLabel) {
        return;
    }
    errorLabel->setText(message);
    errorLabel->setVisible(true);
    adjustSize();
}

void DlgCreateGame::clearError()
{
    if (!errorLabel) {
        return;
    }
    errorLabel->clear();
    errorLabel->setVisible(false);
    adjustSize();
}
