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
    hideBuddiesOnlyGames = new QCheckBox(tr("Hide 'buddies only' games"));
    hideBuddiesOnlyGames->setChecked(gamesProxyModel->getHideBuddiesOnlyGames());

    hideFullGames = new QCheckBox(tr("Hide full games"));
    hideFullGames->setChecked(gamesProxyModel->getHideFullGames());

    hideGamesThatStarted = new QCheckBox(tr("Hide games that have started"));
    hideGamesThatStarted->setChecked(gamesProxyModel->getHideGamesThatStarted());

    hidePasswordProtectedGames = new QCheckBox(tr("Hide password protected games"));
    hidePasswordProtectedGames->setChecked(gamesProxyModel->getHidePasswordProtectedGames());

    hideIgnoredUserGames = new QCheckBox(tr("Hide 'ignored user' games"));
    hideIgnoredUserGames->setChecked(gamesProxyModel->getHideIgnoredUserGames());

    hideNotBuddyCreatedGames = new QCheckBox(tr("Hide games not created by buddy"));
    hideNotBuddyCreatedGames->setChecked(gamesProxyModel->getHideNotBuddyCreatedGames());

    hideOpenDecklistGames = new QCheckBox(tr("Hide games with forced open decklists"));
    hideOpenDecklistGames->setChecked(gamesProxyModel->getHideOpenDecklistGames());

    maxGameAgeComboBox = new QComboBox();
    maxGameAgeComboBox->setEditable(false);
    maxGameAgeComboBox->addItems(gameAgeMap.values());
    QTime gameAge = gamesProxyModel->getMaxGameAge();
    maxGameAgeComboBox->setCurrentIndex(gameAgeMap.keys().indexOf(gameAge)); // index is -1 if unknown
    auto *maxGameAgeLabel = new QLabel(tr("&Newer than:"));
    maxGameAgeLabel->setBuddy(maxGameAgeComboBox);

    gameNameFilterEdit = new QLineEdit;
    gameNameFilterEdit->setText(gamesProxyModel->getGameNameFilter());
    auto *gameNameFilterLabel = new QLabel(tr("Game &description:"));
    gameNameFilterLabel->setBuddy(gameNameFilterEdit);
    creatorNameFilterEdit = new QLineEdit;
    creatorNameFilterEdit->setText(gamesProxyModel->getCreatorNameFilter());
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
        temp->setChecked(gamesProxyModel->getGameTypeFilter().contains(gameTypesIterator.key()));

        gameTypeFilterCheckBoxes.insert(gameTypesIterator.key(), temp);
        gameTypeFilterLayout->addWidget(temp);
    }
    QGroupBox *gameTypeFilterGroupBox;
    if (!allGameTypes.isEmpty()) {
        gameTypeFilterGroupBox = new QGroupBox(tr("&Game types"));
        gameTypeFilterGroupBox->setLayout(gameTypeFilterLayout);
    } else
        gameTypeFilterGroupBox = nullptr;

    auto *maxPlayersFilterMinLabel = new QLabel(tr("at &least:"));
    maxPlayersFilterMinSpinBox = new QSpinBox;
    maxPlayersFilterMinSpinBox->setMinimum(0);
    maxPlayersFilterMinSpinBox->setMaximum(99);
    maxPlayersFilterMinSpinBox->setValue(gamesProxyModel->getMaxPlayersFilterMin());
    maxPlayersFilterMinLabel->setBuddy(maxPlayersFilterMinSpinBox);

    auto *maxPlayersFilterMaxLabel = new QLabel(tr("at &most:"));
    maxPlayersFilterMaxSpinBox = new QSpinBox;
    maxPlayersFilterMaxSpinBox->setMinimum(0);
    maxPlayersFilterMaxSpinBox->setMaximum(99);
    maxPlayersFilterMaxSpinBox->setValue(gamesProxyModel->getMaxPlayersFilterMax());
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
    showOnlyIfSpectatorsCanWatch->setChecked(gamesProxyModel->getShowOnlyIfSpectatorsCanWatch());
    connect(showOnlyIfSpectatorsCanWatch, &QCheckBox::toggled, this,
            &DlgFilterGames::toggleSpectatorCheckboxEnabledness);

    showSpectatorPasswordProtected = new QCheckBox(tr("Show spectator password p&rotected games"));
    showSpectatorPasswordProtected->setChecked(gamesProxyModel->getShowSpectatorPasswordProtected());
    showOnlyIfSpectatorsCanChat = new QCheckBox(tr("Show only if spectators can ch&at"));
    showOnlyIfSpectatorsCanChat->setChecked(gamesProxyModel->getShowOnlyIfSpectatorsCanChat());
    showOnlyIfSpectatorsCanSeeHands = new QCheckBox(tr("Show only if spectators can see &hands"));
    showOnlyIfSpectatorsCanSeeHands->setChecked(gamesProxyModel->getShowOnlyIfSpectatorsCanSeeHands());
    toggleSpectatorCheckboxEnabledness(getShowOnlyIfSpectatorsCanWatch());

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

bool DlgFilterGames::getHideFullGames() const
{
    return hideFullGames->isChecked();
}

bool DlgFilterGames::getHideGamesThatStarted() const
{
    return hideGamesThatStarted->isChecked();
}

bool DlgFilterGames::getHideBuddiesOnlyGames() const
{
    return hideBuddiesOnlyGames->isChecked();
}

bool DlgFilterGames::getHidePasswordProtectedGames() const
{
    return hidePasswordProtectedGames->isChecked();
}

bool DlgFilterGames::getHideIgnoredUserGames() const
{
    return hideIgnoredUserGames->isChecked();
}

bool DlgFilterGames::getHideNotBuddyCreatedGames() const
{
    return hideNotBuddyCreatedGames->isChecked();
}

bool DlgFilterGames::getHideOpenDecklistGames() const
{
    return hideOpenDecklistGames->isChecked();
}

QString DlgFilterGames::getGameNameFilter() const
{
    return gameNameFilterEdit->text();
}

QString DlgFilterGames::getCreatorNameFilter() const
{
    return creatorNameFilterEdit->text();
}

QSet<int> DlgFilterGames::getGameTypeFilter() const
{
    QSet<int> result;
    QMapIterator<int, QCheckBox *> i(gameTypeFilterCheckBoxes);
    while (i.hasNext()) {
        i.next();
        if (i.value()->isChecked())
            result.insert(i.key());
    }
    return result;
}

int DlgFilterGames::getMaxPlayersFilterMin() const
{
    return maxPlayersFilterMinSpinBox->value();
}

int DlgFilterGames::getMaxPlayersFilterMax() const
{
    return maxPlayersFilterMaxSpinBox->value();
}

QTime DlgFilterGames::getMaxGameAge() const
{
    int index = maxGameAgeComboBox->currentIndex();
    if (index < 0 || index >= gameAgeMap.size()) { // index is out of bounds
        return gamesProxyModel->getMaxGameAge();   // leave the setting unchanged
    }
    return gameAgeMap.keys().at(index);
}

bool DlgFilterGames::getShowOnlyIfSpectatorsCanWatch() const
{
    return showOnlyIfSpectatorsCanWatch->isChecked();
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
