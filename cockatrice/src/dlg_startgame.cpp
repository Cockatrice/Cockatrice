#include <QtGui>
#include <QFileDialog>
#include "dlg_startgame.h"
#include "decklistmodel.h"
#include "carddatabase.h"
#include "main.h"

DlgStartGame::DlgStartGame(QWidget *parent)
	: QDialog(parent)
{
	deckView = new QTreeView;
	deckModel = new DeckListModel(this);
	deckView->setModel(deckModel);
	deckView->setUniformRowHeights(true);

	loadButton = new QPushButton(tr("&Load..."));
	okButton = new QPushButton(tr("&OK"));
	okButton->setDefault(true);

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(loadButton);
	buttonLayout->addStretch();
	buttonLayout->addWidget(okButton);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(deckView);
	mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);

	setWindowTitle(tr("Start game"));
	setMinimumWidth(sizeHint().width());
	resize(300, 500);

	connect(loadButton, SIGNAL(clicked()), this, SLOT(actLoad()));
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
}

void DlgStartGame::actLoad()
{
//	if (!deckModel->getDeckList()->loadDialog(this))
//		return;

	deckView->reset();
	deckModel->sort(1);
	deckView->expandAll();
	deckView->resizeColumnToContents(0);
	
	emit newDeckLoaded(getDeckList());
}

QStringList DlgStartGame::getDeckList() const
{
	QStringList result;
	DeckList *deckList = deckModel->getDeckList();
	for (int i = 0; i < deckList->getRoot()->size(); i++) {
		InnerDecklistNode *node = dynamic_cast<InnerDecklistNode *>(deckList->getRoot()->at(i));
		for (int j = 0; j < node->size(); j++) {
			DecklistCardNode *card = dynamic_cast<DecklistCardNode *>(node->at(j));
			for (int k = 0; k < card->getNumber(); k++)
				result << QString("%1%2").arg(node->getName() == "side" ? "SB: " : "").arg(card->getName());
		}
	}
	return result;
}
