#include "dlg_filter_games.h"
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QDialogButtonBox>

DlgFilterGames::DlgFilterGames(const QMap<int, QString> &allGameTypes, QWidget *parent)
	: QDialog(parent)
{
	unavailableGamesVisibleCheckBox = new QCheckBox(tr("Show &unavailable games"));
	
	QLabel *gameNameFilterLabel = new QLabel(tr("Game &description:"));
	gameNameFilterEdit = new QLineEdit;
	
	QLabel *creatorNameFilterLabel = new QLabel(tr("&Creator name:"));
	creatorNameFilterEdit = new QLineEdit;
	
	QVBoxLayout *gameTypeFilterLayout = new QVBoxLayout;
	QMapIterator<int, QString> gameTypesIterator(allGameTypes);
	while (gameTypesIterator.hasNext()) {
		gameTypesIterator.next();
		QCheckBox *temp = new QCheckBox(gameTypesIterator.value());
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
	maxPlayersFilterMinSpinBox->setValue(1);
	maxPlayersFilterMinLabel->setBuddy(maxPlayersFilterMinSpinBox);
	
	QLabel *maxPlayersFilterMaxLabel = new QLabel(tr("at &most:"));
	maxPlayersFilterMaxSpinBox = new QSpinBox;
	maxPlayersFilterMaxSpinBox->setMinimum(1);
	maxPlayersFilterMaxSpinBox->setMaximum(99);
	maxPlayersFilterMaxSpinBox->setValue(99);
	maxPlayersFilterMaxLabel->setBuddy(maxPlayersFilterMaxSpinBox);
	
	QGridLayout *maxPlayersFilterLayout = new QGridLayout;
	maxPlayersFilterLayout->addWidget(maxPlayersFilterMinLabel, 0, 0);
	maxPlayersFilterLayout->addWidget(maxPlayersFilterMinSpinBox, 0, 1);
	maxPlayersFilterLayout->addWidget(maxPlayersFilterMaxLabel, 1, 0);
	maxPlayersFilterLayout->addWidget(maxPlayersFilterMaxSpinBox, 1, 1);
	
	QGroupBox *maxPlayersGroupBox = new QGroupBox(tr("Maximum player count"));
	maxPlayersGroupBox->setLayout(maxPlayersFilterLayout);
	
	QGridLayout *leftColumn = new QGridLayout;
	leftColumn->addWidget(gameNameFilterLabel, 0, 0);
	leftColumn->addWidget(gameNameFilterEdit, 0, 1);
	leftColumn->addWidget(creatorNameFilterLabel, 1, 0);
	leftColumn->addWidget(creatorNameFilterEdit, 1, 1);
	leftColumn->addWidget(maxPlayersGroupBox, 2, 0, 1, 2);
	
	QVBoxLayout *rightColumn = new QVBoxLayout;
	rightColumn->addWidget(gameTypeFilterGroupBox);
	
	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addLayout(leftColumn);
	hbox->addLayout(rightColumn);
	
	QPushButton *okButton = new QPushButton(tr("O&K"));
	okButton->setDefault(true);
	okButton->setAutoDefault(true);
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	QPushButton *cancelButton = new QPushButton(tr("&Cancel"));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(hbox);
	mainLayout->addWidget(buttonBox);
	
	setLayout(mainLayout);
	setWindowTitle(tr("Filter games"));
}

bool DlgFilterGames::getUnavailableGamesVisible() const
{
	return unavailableGamesVisibleCheckBox->isChecked();
}

void DlgFilterGames::setUnavailableGamesVisible(bool _unavailableGamesVisible)
{
	unavailableGamesVisibleCheckBox->setChecked(_unavailableGamesVisible);
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

void DlgFilterGames::setMaxPlayersFilter(int _maxPlayersFilterMin, int _maxPlayersFilterMax)
{
	maxPlayersFilterMinSpinBox->setValue(_maxPlayersFilterMin);
	maxPlayersFilterMaxSpinBox->setValue(_maxPlayersFilterMax == -1 ? maxPlayersFilterMaxSpinBox->maximum() : _maxPlayersFilterMax);
}
