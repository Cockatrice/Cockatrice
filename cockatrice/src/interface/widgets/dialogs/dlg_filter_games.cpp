#include "dlg_filter_games.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCryptographicHash>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

DlgFilterGames::DlgFilterGames(const QMap<int, QString> &_allGameTypes,
                               const GamesProxyModel *_gamesProxyModel,
                               QWidget *parent)
    : QDialog(parent), allGameTypes(_allGameTypes), gamesProxyModel(_gamesProxyModel),
      gameAgeMap({{QTime(), tr("no limit")},
                  {QTime(0, 5), tr("5 minutes")},
                  {QTime(0, 10), tr("10 minutes")},
                  {QTime(0, 30), tr("30 minutes")},
                  {QTime(1, 0), tr("1 hour")},
                  {QTime(2, 0), tr("2 hours")}})
{
    const GameFilterConfigs &filters = gamesProxyModel->getFilters();

    hideBuddiesOnlyGames = new QCheckBox(tr("Hide 'buddies only' games"));
    hideBuddiesOnlyGames->setChecked(filters.hideBuddiesOnlyGames);

    hideFullGames = new QCheckBox(tr("Hide full games"));
    hideFullGames->setChecked(filters.hideFullGames);

    hideGamesThatStarted = new QCheckBox(tr("Hide games that have started"));
    hideGamesThatStarted->setChecked(filters.hideGamesThatStarted);

    hidePasswordProtectedGames = new QCheckBox(tr("Hide password protected games"));
    hidePasswordProtectedGames->setChecked(filters.hidePasswordProtectedGames);

    hideIgnoredUserGames = new QCheckBox(tr("Hide 'ignored user' games"));
    hideIgnoredUserGames->setChecked(filters.hideIgnoredUserGames);

    hideNotBuddyCreatedGames = new QCheckBox(tr("Hide games not created by buddies"));
    hideNotBuddyCreatedGames->setChecked(filters.hideNotBuddyCreatedGames);

    hideOpenDecklistGames = new QCheckBox(tr("Hide games with forced open decklists"));
    hideOpenDecklistGames->setChecked(filters.hideOpenDecklistGames);

    maxGameAgeComboBox = new QComboBox();
    maxGameAgeComboBox->setEditable(false);
    maxGameAgeComboBox->addItems(gameAgeMap.values());
    QTime gameAge = filters.maxGameAge;
    maxGameAgeComboBox->setCurrentIndex(gameAgeMap.keys().indexOf(gameAge)); // index is -1 if unknown
    auto *maxGameAgeLabel = new QLabel(tr("&Newer than:"));
    maxGameAgeLabel->setBuddy(maxGameAgeComboBox);

    gameNameFilterEdit = new QLineEdit;
    gameNameFilterEdit->setText(filters.gameNameFilter);
    auto *gameNameFilterLabel = new QLabel(tr("Game &description:"));
    gameNameFilterLabel->setBuddy(gameNameFilterEdit);
    creatorNameFilterEdit = new QLineEdit;
    creatorNameFilterEdit->setText(filters.creatorNameFilters.join(", "));
    auto *creatorNameFilterLabel = new QLabel(tr("&Creator name:"));
    creatorNameFilterLabel->setBuddy(creatorNameFilterEdit);

    auto *generalGrid = new QGridLayout;
    generalGrid->addWidget(gameNameFilterLabel, 0, 0);
    generalGrid->addWidget(gameNameFilterEdit, 0, 1);
    generalGrid->addWidget(creatorNameFilterLabel, 1, 0);
    generalGrid->addWidget(creatorNameFilterEdit, 1, 1);
    generalGrid->addWidget(maxGameAgeLabel, 2, 0);
    generalGrid->addWidget(maxGameAgeComboBox, 2, 1);
    generalGroupBox = new QGroupBox(tr("General"));
    generalGroupBox->setLayout(generalGrid);

    auto *gameTypeFilterLayout = new QVBoxLayout;
    QMapIterator<int, QString> gameTypesIterator(allGameTypes);
    while (gameTypesIterator.hasNext()) {
        gameTypesIterator.next();

        auto *temp = new QCheckBox(gameTypesIterator.value());
        temp->setChecked(filters.gameTypeFilter.contains(gameTypesIterator.key()));

        gameTypeFilterCheckBoxes.insert(gameTypesIterator.key(), temp);
        gameTypeFilterLayout->addWidget(temp);
    }
    QGroupBox *gameTypeFilterGroupBox;
    if (!allGameTypes.isEmpty()) {
        gameTypeFilterGroupBox = new QGroupBox(tr("&Game types"));
        gameTypeFilterGroupBox->setLayout(gameTypeFilterLayout);
    } else {
        gameTypeFilterGroupBox = nullptr;
    }

    auto *maxPlayersFilterMinLabel = new QLabel(tr("at &least:"));
    maxPlayersFilterMinSpinBox = new QSpinBox;
    maxPlayersFilterMinSpinBox->setMinimum(0);
    maxPlayersFilterMinSpinBox->setMaximum(99);
    maxPlayersFilterMinSpinBox->setValue(filters.maxPlayersFilterMin);
    maxPlayersFilterMinLabel->setBuddy(maxPlayersFilterMinSpinBox);

    auto *maxPlayersFilterMaxLabel = new QLabel(tr("at &most:"));
    maxPlayersFilterMaxSpinBox = new QSpinBox;
    maxPlayersFilterMaxSpinBox->setMinimum(0);
    maxPlayersFilterMaxSpinBox->setMaximum(99);
    maxPlayersFilterMaxSpinBox->setValue(filters.maxPlayersFilterMax);
    maxPlayersFilterMaxLabel->setBuddy(maxPlayersFilterMaxSpinBox);

    auto *maxPlayersFilterLayout = new QGridLayout;
    maxPlayersFilterLayout->addWidget(maxPlayersFilterMinLabel, 0, 0);
    maxPlayersFilterLayout->addWidget(maxPlayersFilterMinSpinBox, 0, 1);
    maxPlayersFilterLayout->addWidget(maxPlayersFilterMaxLabel, 1, 0);
    maxPlayersFilterLayout->addWidget(maxPlayersFilterMaxSpinBox, 1, 1);

    auto *maxPlayersGroupBox = new QGroupBox(tr("Maximum player count"));
    maxPlayersGroupBox->setLayout(maxPlayersFilterLayout);

    auto *restrictionsLayout = new QGridLayout;
    restrictionsLayout->addWidget(hideFullGames, 0, 0);
    restrictionsLayout->addWidget(hideGamesThatStarted, 1, 0);
    restrictionsLayout->addWidget(hidePasswordProtectedGames, 2, 0);
    restrictionsLayout->addWidget(hideBuddiesOnlyGames, 3, 0);
    restrictionsLayout->addWidget(hideIgnoredUserGames, 4, 0);
    restrictionsLayout->addWidget(hideNotBuddyCreatedGames, 5, 0);
    restrictionsLayout->addWidget(hideOpenDecklistGames, 6, 0);

    auto *restrictionsGroupBox = new QGroupBox(tr("Restrictions"));
    restrictionsGroupBox->setLayout(restrictionsLayout);

    showOnlyIfSpectatorsCanWatch = new QCheckBox(tr("Show games only if &spectators can watch"));
    showOnlyIfSpectatorsCanWatch->setChecked(filters.showOnlyIfSpectatorsCanWatch);
    connect(showOnlyIfSpectatorsCanWatch, &QCheckBox::toggled, this,
            &DlgFilterGames::toggleSpectatorCheckboxEnabledness);

    showSpectatorPasswordProtected = new QCheckBox(tr("Show spectator password p&rotected games"));
    showSpectatorPasswordProtected->setChecked(filters.showSpectatorPasswordProtected);
    showOnlyIfSpectatorsCanChat = new QCheckBox(tr("Show only if spectators can ch&at"));
    showOnlyIfSpectatorsCanChat->setChecked(filters.showOnlyIfSpectatorsCanChat);
    showOnlyIfSpectatorsCanSeeHands = new QCheckBox(tr("Show only if spectators can see &hands"));
    showOnlyIfSpectatorsCanSeeHands->setChecked(filters.showOnlyIfSpectatorsCanSeeHands);
    toggleSpectatorCheckboxEnabledness(filters.showOnlyIfSpectatorsCanWatch);

    auto *spectatorsLayout = new QGridLayout;
    spectatorsLayout->addWidget(showOnlyIfSpectatorsCanWatch, 0, 0);
    spectatorsLayout->addWidget(showSpectatorPasswordProtected, 1, 0);
    spectatorsLayout->addWidget(showOnlyIfSpectatorsCanChat, 2, 0);
    spectatorsLayout->addWidget(showOnlyIfSpectatorsCanSeeHands, 3, 0);

    auto *spectatorsGroupBox = new QGroupBox(tr("Spectators"));
    spectatorsGroupBox->setLayout(spectatorsLayout);

    auto *leftGrid = new QGridLayout;
    leftGrid->addWidget(generalGroupBox, 0, 0, 1, 2);
    leftGrid->addWidget(maxPlayersGroupBox, 2, 0, 1, 2);
    leftGrid->addWidget(restrictionsGroupBox, 3, 0, 1, 2);

    auto *leftColumn = new QVBoxLayout;
    leftColumn->addLayout(leftGrid);
    leftColumn->addStretch();

    auto *rightGrid = new QGridLayout;
    rightGrid->addWidget(gameTypeFilterGroupBox, 0, 0, 1, 1);
    rightGrid->addWidget(spectatorsGroupBox, 1, 0, 1, 1);

    auto *rightColumn = new QVBoxLayout;
    rightColumn->addLayout(rightGrid);
    rightColumn->addStretch();

    auto *hbox = new QHBoxLayout;
    hbox->addLayout(leftColumn);
    hbox->addLayout(rightColumn);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgFilterGames::actOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgFilterGames::reject);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(hbox);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("Filter games"));

    setFixedHeight(sizeHint().height());
}

GameFilterConfigs DlgFilterGames::getFilters() const
{
    return {hideBuddiesOnlyGames->isChecked(),
            hideIgnoredUserGames->isChecked(),
            hideFullGames->isChecked(),
            hideGamesThatStarted->isChecked(),
            hidePasswordProtectedGames->isChecked(),
            hideNotBuddyCreatedGames->isChecked(),
            hideOpenDecklistGames->isChecked(),
            gameNameFilterEdit->text(),
            getCreatorNameFilters(),
            getGameTypeFilter(),
            maxPlayersFilterMinSpinBox->value(),
            maxPlayersFilterMaxSpinBox->value(),
            getMaxGameAge(),
            showOnlyIfSpectatorsCanWatch->isChecked(),
            getShowSpectatorPasswordProtected(),
            getShowOnlyIfSpectatorsCanChat(),
            getShowOnlyIfSpectatorsCanSeeHands()};
}

void DlgFilterGames::actOk()
{
    accept();
}

void DlgFilterGames::toggleSpectatorCheckboxEnabledness(bool spectatorsEnabled)
{
    showSpectatorPasswordProtected->setDisabled(!spectatorsEnabled);
    showOnlyIfSpectatorsCanChat->setDisabled(!spectatorsEnabled);
    showOnlyIfSpectatorsCanSeeHands->setDisabled(!spectatorsEnabled);
}

QStringList DlgFilterGames::getCreatorNameFilters() const
{
    return creatorNameFilterEdit->text().split(",", Qt::SkipEmptyParts);
}

QSet<int> DlgFilterGames::getGameTypeFilter() const
{
    QSet<int> result;
    QMapIterator<int, QCheckBox *> i(gameTypeFilterCheckBoxes);
    while (i.hasNext()) {
        i.next();
        if (i.value()->isChecked()) {
            result.insert(i.key());
        }
    }
    return result;
}

QTime DlgFilterGames::getMaxGameAge() const
{
    int index = maxGameAgeComboBox->currentIndex();
    if (index < 0 || index >= gameAgeMap.size()) {       // index is out of bounds
        return gamesProxyModel->getFilters().maxGameAge; // leave the setting unchanged
    }
    return gameAgeMap.keys().at(index);
}

bool DlgFilterGames::getShowSpectatorPasswordProtected() const
{
    return showSpectatorPasswordProtected->isEnabled() && showSpectatorPasswordProtected->isChecked();
}

bool DlgFilterGames::getShowOnlyIfSpectatorsCanChat() const
{
    return showOnlyIfSpectatorsCanChat->isEnabled() && showOnlyIfSpectatorsCanChat->isChecked();
}

bool DlgFilterGames::getShowOnlyIfSpectatorsCanSeeHands() const
{
    return showOnlyIfSpectatorsCanSeeHands->isEnabled() && showOnlyIfSpectatorsCanSeeHands->isChecked();
}
