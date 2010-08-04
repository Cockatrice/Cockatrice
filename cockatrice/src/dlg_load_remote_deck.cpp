#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "remotedecklist_treewidget.h"
#include "dlg_load_remote_deck.h"
#include "main.h"

DlgLoadRemoteDeck::DlgLoadRemoteDeck(AbstractClient *_client, QWidget *parent)
	: QDialog(parent), client(_client)
{
	dirView = new RemoteDeckList_TreeWidget(client);

	okButton = new QPushButton(tr("O&K"));
	okButton->setDefault(true);
	okButton->setAutoDefault(true);
	okButton->setEnabled(false);
	cancelButton = new QPushButton(tr("&Cancel"));

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch();
	buttonLayout->addWidget(okButton);
	buttonLayout->addWidget(cancelButton);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(dirView);
	mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);

	setWindowTitle(tr("Load deck"));
	setMinimumWidth(sizeHint().width());
	resize(300, 500);

	connect(dirView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(currentItemChanged(const QModelIndex &, const QModelIndex &)));
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void DlgLoadRemoteDeck::currentItemChanged(const QModelIndex &current, const QModelIndex & /*previous*/)
{
	okButton->setEnabled(dynamic_cast<RemoteDeckList_TreeModel::FileNode *>(dirView->getNode(current)));
}

int DlgLoadRemoteDeck::getDeckId() const
{
	return dynamic_cast<RemoteDeckList_TreeModel::FileNode *>(dirView->getNode(dirView->selectionModel()->currentIndex()))->getId();
}
