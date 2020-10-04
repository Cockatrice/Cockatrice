#include "dlg_filter_games.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCryptographicHash>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

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
    showBuddiesOnlyGames = new QCheckBox(tr("Show '&buddies only' games"));
    showBuddiesOnlyGames->setChecked(gamesProxyModel->getShowBuddiesOnlyGames());

    unavailableGamesVisibleCheckBox = new QCheckBox(tr("Show &unavailable games"));
    unavailableGamesVisibleCheckBox->setChecked(gamesProxyModel->getUnavailableGamesVisible());

    showPasswordProtectedGames = new QCheckBox(tr("Show &password protected games"));
    showPasswordProtectedGames->setChecked(gamesProxyModel->getShowPasswordProtectedGames());

    hideIgnoredUserGames = new QCheckBox(tr("Hide '&ignored user' games"));
    hideIgnoredUserGames->setChecked(gamesProxyModel->getHideIgnoredUserGames());

    maxGameAgeComboBox = new QComboBox();
    maxGameAgeComboBox->setEditable(false);
    maxGameAgeComboBox->addItems(gameAgeMap.values());
    QTime gameAge = gamesProxyModel->getMaxGameAge();
    maxGameAgeComboBox->setCurrentIndex(gameAgeMap.keys().indexOf(gameAge)); // index is -1 if unknown
    QLabel *maxGameAgeLabel = new QLabel(tr("&Newer than:"));
    maxGameAgeLabel->setBuddy(maxGameAgeComboBox);

    gameNameFilterEdit = new QLineEdit;
    gameNameFilterEdit->setText(gamesProxyModel->getGameNameFilter());
    QLabel *gameNameFilterLabel = new QLabel(tr("Game &description:"));
    gameNameFilterLabel->setBuddy(gameNameFilterEdit);
    creatorNameFilterEdit = new QLineEdit;
    creatorNameFilterEdit->setText(gamesProxyModel->getCreatorNameFilter());
    QLabel *creatorNameFilterLabel = new QLabel(tr("&Creator name:"));
    creatorNameFilterLabel->setBuddy(creatorNameFilterEdit);

    QGridLayout *generalGrid = new QGridLayout;
    generalGrid->addWidget(gameNameFilterLabel, 0, 0);
    generalGrid->addWidget(gameNameFilterEdit, 0, 1);
    generalGrid->addWidget(creatorNameFilterLabel, 1, 0);
    generalGrid->addWidget(creatorNameFilterEdit, 1, 1);
    generalGrid->addWidget(maxGameAgeLabel, 2, 0);
    generalGrid->addWidget(maxGameAgeComboBox, 2, 1);
    generalGroupBox = new QGroupBox(tr("General"));
    generalGroupBox->setLayout(generalGrid);

    QVBoxLayout *gameTypeFilterLayout = new QVBoxLayout;
    QMapIterator<int, QString> gameTypesIterator(allGameTypes);
    while (gameTypesIterator.hasNext()) {
        gameTypesIterator.next();

        QCheckBox *temp = new QCheckBox(gameTypesIterator.value());
        temp->setChecked(gamesProxyModel->getGameTypeFilter().contains(gameTypesIterator.key()));

        gameTypeFilterCheckBoxes.insert(gameTypesIterator.key(), temp);
        gameTypeFilterLayout->addWidget(temp);
    }
    QGroupBox *gameTypeFilterGroupBox;
    if (!allGameTypes.isEmpty()) {
        gameTypeFilterGroupBox = new QGroupBox(tr("&Game types"));
        gameTypeFilterGroupBox->setLayout(gameTypeFilterLayout);
    } else
        gameTypeFilterGroupBox = 0;

    QLabel *maxPlayersFilterMinLabel = new QLabel(tr("at &least:"));
    maxPlayersFilterMinSpinBox = new QSpinBox;
    maxPlayersFilterMinSpinBox->setMinimum(1);
    maxPlayersFilterMinSpinBox->setMaximum(99);
    maxPlayersFilterMinSpinBox->setValue(gamesProxyModel->getMaxPlayersFilterMin());
    maxPlayersFilterMinLabel->setBuddy(maxPlayersFilterMinSpinBox);

    QLabel *maxPlayersFilterMaxLabel = new QLabel(tr("at &most:"));
    maxPlayersFilterMaxSpinBox = new QSpinBox;
    maxPlayersFilterMaxSpinBox->setMinimum(1);
    maxPlayersFilterMaxSpinBox->setMaximum(99);
    maxPlayersFilterMaxSpinBox->setValue(gamesProxyModel->getMaxPlayersFilterMax());
    maxPlayersFilterMaxLabel->setBuddy(maxPlayersFilterMaxSpinBox);

    QGridLayout *maxPlayersFilterLayout = new QGridLayout;
    maxPlayersFilterLayout->addWidget(maxPlayersFilterMinLabel, 0, 0);
    maxPlayersFilterLayout->addWidget(maxPlayersFilterMinSpinBox, 0, 1);
    maxPlayersFilterLayout->addWidget(maxPlayersFilterMaxLabel, 1, 0);
    maxPlayersFilterLayout->addWidget(maxPlayersFilterMaxSpinBox, 1, 1);

    QGroupBox *maxPlayersGroupBox = new QGroupBox(tr("Maximum player count"));
    maxPlayersGroupBox->setLayout(maxPlayersFilterLayout);

    QGridLayout *restrictionsLayout = new QGridLayout;
    restrictionsLayout->addWidget(unavailableGamesVisibleCheckBox, 0, 0);
    restrictionsLayout->addWidget(showPasswordProtectedGames, 1, 0);
    restrictionsLayout->addWidget(showBuddiesOnlyGames, 2, 0);
    restrictionsLayout->addWidget(hideIgnoredUserGames, 3, 0);

    QGroupBox *restrictionsGroupBox = new QGroupBox(tr("Restrictions"));
    restrictionsGroupBox->setLayout(restrictionsLayout);

    spectatorsCanWatch = new QCheckBox(tr("Show games &spectators can watch"));
    spectatorsCanWatch->setChecked(true);  // DO NOT SUBMIT get from storage
    connect(spectatorsCanWatch, SIGNAL(toggled(bool)), this, SLOT(toggleSpectatorCheckboxEnabledness(bool)));

    spectatorsNeedPassword = new QCheckBox(tr("Show if spectators need password"));
    spectatorsNeedPassword->setChecked(true);  // DO NOT SUBMIT get from storage
    spectatorsCanChat = new QCheckBox(tr("Show if spectators can chat"));
    spectatorsCanChat->setChecked(true);  // DO NOT SUBMIT get from storage
    spectatorsCanSeeHands = new QCheckBox(tr("Show if spectators can see hands"));
    spectatorsCanSeeHands->setChecked(true);  // DO NOT SUBMIT get from storage
    toggleSpectatorCheckboxEnabledness(getSpectatorsCanWatch());

    QGridLayout *spectatorsLayout = new QGridLayout;
    spectatorsLayout->addWidget(spectatorsCanWatch, 0, 0);
    spectatorsLayout->addWidget(spectatorsNeedPassword, 1, 0);
    spectatorsLayout->addWidget(spectatorsCanChat, 2, 0);
    spectatorsLayout->addWidget(spectatorsCanSeeHands, 3, 0);

    QGroupBox *spectatorsGroupBox = new QGroupBox(tr("Spectators"));
    spectatorsGroupBox->setLayout(spectatorsLayout);

    QGridLayout *leftGrid = new QGridLayout;
    leftGrid->addWidget(generalGroupBox, 0, 0, 1, 2);
    leftGrid->addWidget(maxPlayersGroupBox, 2, 0, 1, 2);
    leftGrid->addWidget(restrictionsGroupBox, 3, 0, 1, 2);
    leftGrid->addWidget(spectatorsGroupBox, 4, 0, 1, 2);

    QVBoxLayout *leftColumn = new QVBoxLayout;
    leftColumn->addLayout(leftGrid);
    leftColumn->addStretch();

    QVBoxLayout *rightColumn = new QVBoxLayout;
    rightColumn->addWidget(gameTypeFilterGroupBox);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addLayout(leftColumn);
    hbox->addLayout(rightColumn);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(hbox);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("Filter games"));
}

void DlgFilterGames::actOk()
{
    accept();
}

void DlgFilterGames::toggleSpectatorCheckboxEnabledness(bool spectatorsEnabled)
{
    spectatorsNeedPassword->setDisabled(!spectatorsEnabled);
    spectatorsCanChat->setDisabled(!spectatorsEnabled);
    spectatorsCanSeeHands->setDisabled(!spectatorsEnabled);
}

bool DlgFilterGames::getUnavailableGamesVisible() const
{
    return unavailableGamesVisibleCheckBox->isChecked();
}

void DlgFilterGames::setUnavailableGamesVisible(bool _unavailableGamesVisible)
{
    unavailableGamesVisibleCheckBox->setChecked(_unavailableGamesVisible);
}

bool DlgFilterGames::getShowBuddiesOnlyGames() const
{
    return showBuddiesOnlyGames->isChecked();
}

void DlgFilterGames::setShowBuddiesOnlyGames(bool _showBuddiesOnlyGames)
{
    showBuddiesOnlyGames->setChecked(_showBuddiesOnlyGames);
}

bool DlgFilterGames::getShowPasswordProtectedGames() const
{
    return showPasswordProtectedGames->isChecked();
}

void DlgFilterGames::setShowPasswordProtectedGames(bool _passwordProtectedGamesHidden)
{
    showPasswordProtectedGames->setChecked(_passwordProtectedGamesHidden);
}

bool DlgFilterGames::getHideIgnoredUserGames() const
{
    return hideIgnoredUserGames->isChecked();
}

void DlgFilterGames::setHideIgnoredUserGames(bool _hideIgnoredUserGames)
{
    hideIgnoredUserGames->setChecked(_hideIgnoredUserGames);
}

QString DlgFilterGames::getGameNameFilter() const
{
    return gameNameFilterEdit->text();
}

void DlgFilterGames::setGameNameFilter(const QString &_gameNameFilter)
{
    gameNameFilterEdit->setText(_gameNameFilter);
}

QString DlgFilterGames::getCreatorNameFilter() const
{
    return creatorNameFilterEdit->text();
}

void DlgFilterGames::setCreatorNameFilter(const QString &_creatorNameFilter)
{
    creatorNameFilterEdit->setText(_creatorNameFilter);
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

void DlgFilterGames::setGameTypeFilter(const QSet<int> &_gameTypeFilter)
{
    QMapIterator<int, QCheckBox *> i(gameTypeFilterCheckBoxes);
    while (i.hasNext()) {
        i.next();
        i.value()->setChecked(_gameTypeFilter.contains(i.key()));
    }
}

int DlgFilterGames::getMaxPlayersFilterMin() const
{
    return maxPlayersFilterMinSpinBox->value();
}

int DlgFilterGames::getMaxPlayersFilterMax() const
{
    return maxPlayersFilterMaxSpinBox->value();
}

const QTime &DlgFilterGames::getMaxGameAge() const
{
    int index = maxGameAgeComboBox->currentIndex();
    if (index < 0 || index >= gameAgeMap.size()) { // index is out of bounds
        return gamesProxyModel->getMaxGameAge();   // leave the setting unchanged
    }
    return gameAgeMap.keys().at(index);
}

void DlgFilterGames::setMaxPlayersFilter(int _maxPlayersFilterMin, int _maxPlayersFilterMax)
{
    maxPlayersFilterMinSpinBox->setValue(_maxPlayersFilterMin);
    maxPlayersFilterMaxSpinBox->setValue(_maxPlayersFilterMax == -1 ? maxPlayersFilterMaxSpinBox->maximum()
                                                                    : _maxPlayersFilterMax);
}

int DlgFilterGames::getSpectatorsCanWatch() const
{
    return spectatorsCanWatch->isEnabled() && spectatorsCanWatch->isChecked();
}