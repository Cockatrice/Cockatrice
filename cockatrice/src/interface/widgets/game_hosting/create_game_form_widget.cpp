#include "create_game_form_widget.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <libcockatrice/utility/string_limits.h>
#include <qnamespace.h>

CreateGameFormWidget::CreateGameFormWidget(const QMap<int, QString> &gameTypes,
                                           bool _registeredUser,
                                           bool judge,
                                           QWidget *parent)
    : QWidget(parent), registeredUser(_registeredUser)
{
    setObjectName("createGameForm");

    rememberGameSettings = new QCheckBox(tr("Re&member settings"));
    auto *descriptionLabel = new QLabel(tr("&Description:"));
    descriptionEdit = new QLineEdit;
    descriptionEdit->setMaxLength(MAX_NAME_LENGTH);
    descriptionLabel->setBuddy(descriptionEdit);

    auto *maxPlayersLabel = new QLabel(tr("P&layers:"));
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
    auto *generalGroupBox = new QGroupBox(tr("General"));
    generalGroupBox->setLayout(generalGrid);

    auto *gameTypeLayout = new QVBoxLayout;
    for (auto it = gameTypes.cbegin(); it != gameTypes.cend(); ++it) {
        auto *gameTypeRadioButton = new QRadioButton(it.value(), this);
        gameTypeLayout->addWidget(gameTypeRadioButton);
        gameTypeCheckBoxes.insert(it.key(), gameTypeRadioButton);
    }
    auto *gameTypeGroupBox = new QGroupBox(tr("Game type"));
    gameTypeGroupBox->setLayout(gameTypeLayout);

    auto *passwordLabel = new QLabel(tr("&Password:"));
    passwordEdit = new QLineEdit;
    passwordEdit->setMaxLength(MAX_NAME_LENGTH);
    passwordLabel->setBuddy(passwordEdit);

    onlyBuddiesCheckBox = new QCheckBox(tr("Only &buddies can join"));
    onlyRegisteredCheckBox = new QCheckBox(tr("Only &registered users can join"));
    if (registeredUser) {
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
    connect(spectatorsAllowedCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &CreateGameFormWidget::spectatorsAllowedChanged);
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
    auto *spectatorsGroupBox = new QGroupBox(tr("Spectators"));
    spectatorsGroupBox->setLayout(spectatorsLayout);

    auto *startingLifeTotalLabel = new QLabel(tr("Starting life total:"));
    startingLifeTotalEdit = new QSpinBox();
    startingLifeTotalEdit->setMinimum(1);
    startingLifeTotalEdit->setMaximum(99999); ///< Arbitrary but we can raise this when people start complaining.
    startingLifeTotalEdit->setValue(20);
    startingLifeTotalLabel->setBuddy(startingLifeTotalEdit);

    shareDecklistsOnLoadCheckBox = new QCheckBox(tr("Open decklists in lobby"));

    shufflePlayersCheckBox = new QCheckBox(tr("Randomize turn order (seats) on each game start"));

    createGameAsJudgeCheckBox = new QCheckBox(tr("Create game as judge"));

    auto *gameSetupOptionsLayout = new QGridLayout;
    gameSetupOptionsLayout->addWidget(startingLifeTotalLabel, 0, 0);
    gameSetupOptionsLayout->addWidget(startingLifeTotalEdit, 0, 1);
    gameSetupOptionsLayout->addWidget(shareDecklistsOnLoadCheckBox, 1, 0, 1, 2);
    gameSetupOptionsLayout->addWidget(shufflePlayersCheckBox, 2, 0, 1, 2);
    if (judge) {
        gameSetupOptionsLayout->addWidget(createGameAsJudgeCheckBox, 3, 0, 1, 2);
    } else {
        createGameAsJudgeCheckBox->setChecked(false);
        createGameAsJudgeCheckBox->setHidden(true);
    }
    auto *gameSetupOptionsGroupBox = new QGroupBox(tr("Game setup options"));
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

    setLayout(grid);
}

void CreateGameFormWidget::setOptions(const GameCreationOptions &options)
{
    descriptionEdit->setText(options.description);
    maxPlayersEdit->setValue(options.maxPlayers);
    passwordEdit->setText(options.password);
    onlyBuddiesCheckBox->setChecked(options.onlyBuddies);
    onlyRegisteredCheckBox->setChecked(options.onlyRegistered);
    spectatorsAllowedCheckBox->setChecked(options.spectatorsAllowed);
    spectatorsNeedPasswordCheckBox->setChecked(options.spectatorsNeedPassword);
    spectatorsCanTalkCheckBox->setChecked(options.spectatorsCanTalk);
    spectatorsSeeEverythingCheckBox->setChecked(options.spectatorsSeeEverything);
    createGameAsJudgeCheckBox->setChecked(options.joinAsJudge);
    createGameAsSpectatorCheckBox->setChecked(options.joinAsSpectator);
    startingLifeTotalEdit->setValue(options.startingLifeTotal);
    shareDecklistsOnLoadCheckBox->setChecked(options.shareDecklistsOnLoad);
    shufflePlayersCheckBox->setChecked(options.shufflePlayers);

    for (auto it = gameTypeCheckBoxes.cbegin(); it != gameTypeCheckBoxes.cend(); ++it) {
        it.value()->setChecked(options.gameTypeIds.contains(it.key()));
    }
}

GameCreationOptions CreateGameFormWidget::options() const
{
    GameCreationOptions options;
    options.description = descriptionEdit->text();
    options.password = passwordEdit->text();
    options.maxPlayers = maxPlayersEdit->value();
    options.onlyBuddies = onlyBuddiesCheckBox->isChecked();
    options.onlyRegistered = onlyRegisteredCheckBox->isChecked();
    options.spectatorsAllowed = spectatorsAllowedCheckBox->isChecked();
    options.spectatorsNeedPassword = spectatorsNeedPasswordCheckBox->isChecked();
    options.spectatorsCanTalk = spectatorsCanTalkCheckBox->isChecked();
    options.spectatorsSeeEverything = spectatorsSeeEverythingCheckBox->isChecked();
    options.joinAsJudge = createGameAsJudgeCheckBox->isChecked();
    options.joinAsSpectator = createGameAsSpectatorCheckBox->isChecked();
    options.startingLifeTotal = startingLifeTotalEdit->value();
    options.shareDecklistsOnLoad = shareDecklistsOnLoadCheckBox->isChecked();
    options.shufflePlayers = shufflePlayersCheckBox->isChecked();

    for (auto it = gameTypeCheckBoxes.cbegin(); it != gameTypeCheckBoxes.cend(); ++it) {
        if (it.value()->isChecked()) {
            options.gameTypeIds.append(it.key());
        }
    }

    return options;
}

QString CreateGameFormWidget::selectedGameTypesSettingString() const
{
    QString gameTypes;
    for (auto it = gameTypeCheckBoxes.cbegin(); it != gameTypeCheckBoxes.cend(); ++it) {
        if (it.value()->isChecked()) {
            gameTypes += it.value()->text() + ", ";
        }
    }
    return gameTypes;
}

void CreateGameFormWidget::resetToDefaults()
{
    GameCreationOptions defaults;
    defaults.onlyRegistered = registeredUser;
    setOptions(defaults);

    // An exclusive button group refuses to uncheck its selected button, which setOptions
    // cannot do, so clear the exclusive flag while resetting the game type selection.
    for (auto *gameTypeCheckBox : gameTypeCheckBoxes) {
        gameTypeCheckBox->setAutoExclusive(false);
        gameTypeCheckBox->setChecked(false);
        gameTypeCheckBox->setAutoExclusive(true);
    }

    descriptionEdit->setFocus();
}

void CreateGameFormWidget::setReadOnly(bool readOnly)
{
    const bool editable = !readOnly;
    rememberGameSettings->setEnabled(editable);
    descriptionEdit->setEnabled(editable);
    maxPlayersEdit->setEnabled(editable);
    passwordEdit->setEnabled(editable);
    onlyBuddiesCheckBox->setEnabled(editable && registeredUser);
    onlyRegisteredCheckBox->setEnabled(editable && registeredUser);
    spectatorsAllowedCheckBox->setEnabled(editable);
    spectatorsNeedPasswordCheckBox->setEnabled(editable);
    spectatorsCanTalkCheckBox->setEnabled(editable);
    spectatorsSeeEverythingCheckBox->setEnabled(editable);
    createGameAsSpectatorCheckBox->setEnabled(editable);
    startingLifeTotalEdit->setEnabled(editable);
    shareDecklistsOnLoadCheckBox->setEnabled(editable);
    shufflePlayersCheckBox->setEnabled(editable);
    for (auto *gameTypeCheckBox : gameTypeCheckBoxes) {
        gameTypeCheckBox->setEnabled(editable);
    }
}

void CreateGameFormWidget::focusDescription()
{
    descriptionEdit->setFocus();
}

bool CreateGameFormWidget::isRememberSettingsChecked() const
{
    return rememberGameSettings->isChecked();
}

void CreateGameFormWidget::setRememberSettingsChecked(bool checked)
{
    rememberGameSettings->setChecked(checked);
}

void CreateGameFormWidget::spectatorsAllowedChanged(QT_STATE_CHANGED_T state)
{
    spectatorsNeedPasswordCheckBox->setEnabled(state);
    spectatorsCanTalkCheckBox->setEnabled(state);
    spectatorsSeeEverythingCheckBox->setEnabled(state);
}
