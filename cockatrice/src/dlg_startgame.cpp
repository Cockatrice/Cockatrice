#include <QtGui>
#include <QFileDialog>
#include "dlg_startgame.h"
#include "decklistmodel.h"
#include "carddatabase.h"

DlgStartGame::DlgStartGame(CardDatabase *_db, QWidget *parent)
	: QDialog(parent), db(_db)
{
	tableView = new QTreeView;
	tableModel = new DeckListModel(db, this);
	tableView->setModel(tableModel);

	loadButton = new QPushButton(tr("&Load..."));
	okButton = new QPushButton(tr("&OK"));
	okButton->setDefault(true);

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(loadButton);
	buttonLayout->addStretch();
	buttonLayout->addWidget(okButton);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tableView);
	mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);

	setWindowTitle(tr("Start game"));
	setMinimumWidth(sizeHint().width());

	connect(loadButton, SIGNAL(clicked()), this, SLOT(actLoad()));
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
}

void DlgStartGame::actLoad()
{
	if (!tableModel->getDeckList()->loadDialog(this))
		return;

	tableView->reset();
	emit newDeckLoaded(getDeckList());
}

QStringList DlgStartGame::getDeckList() const
{
	QStringList result;
	DeckList *deckList = tableModel->getDeckList();
	for (int i = 0; i < deckList->zoneCount(); i++) {
		DecklistZone *zone = deckList->getZoneByIndex(i);
		for (int j = 0; j < zone->size(); j++) {
			DecklistRow *r = zone->at(j);
			for (int k = 0; k < r->getNumber(); k++)
				result << QString("%1%2").arg(zone->getName() == "side" ? "SB:" : "").arg(r->getCard());
		}
	}
	return result;
}
