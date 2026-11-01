#include "dlg_tournament_settings.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QPushButton>
#include <QSpinBox>

DlgTournamentSettings::DlgTournamentSettings(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Tournament Settings"));

    auto *mainLayout = new QFormLayout(this);

    gamesPerMatchSpin = new QSpinBox(this);
    gamesPerMatchSpin->setRange(1, 5);
    gamesPerMatchSpin->setValue(1);
    gamesPerMatchSpin->setToolTip(tr("Number of games per match (e.g., 3 for Best of 3, 5 for Best of 5)"));
    mainLayout->addRow(tr("Games per match:"), gamesPerMatchSpin);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgTournamentSettings::actOK);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addRow(buttonBox);

    setFixedHeight(sizeHint().height());
}

DlgTournamentSettingsResult DlgTournamentSettings::getResult() const
{
    DlgTournamentSettingsResult result;
    result.gamesPerMatch = gamesPerMatchSpin->value();
    return result;
}

void DlgTournamentSettings::setCurrentGamesPerMatch(int n)
{
    gamesPerMatchSpin->setValue(n);
}

void DlgTournamentSettings::actOK()
{
    accept();
}
