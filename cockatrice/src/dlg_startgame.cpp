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
	QString fileName = QFileDialog::getOpenFileName(this, tr("Load deck"), QString(), tr("Deck files (*.dec)"));
	if (fileName.isEmpty())
		return;
	tableModel->loadFromFile(fileName);
	
	emit newDeckLoaded(getDeckList());
}

QStringList DlgStartGame::getDeckList() const
{
	QStringList result;
	for (int i = 0; i < tableModel->rowCount(); i++) {
		DecklistRow *temp = tableModel->getRow(i);
		for (int j = 0; j < temp->getNumber(); j++)
			result << QString("%1%2").arg(temp->isSideboard() ? "SB:" : "").arg(temp->getCard());
	}
	return result;
}
