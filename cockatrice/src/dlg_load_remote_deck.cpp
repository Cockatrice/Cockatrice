#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "remotedecklist_treewidget.h"
#include "dlg_load_remote_deck.h"
#include "main.h"

DlgLoadRemoteDeck::DlgLoadRemoteDeck(Client *_client, QWidget *parent)
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

	connect(dirView, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void DlgLoadRemoteDeck::currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem * /*previous*/)
{
	if (!current)
		okButton->setEnabled(false);
	else if (current->type() == TWIDeckType)
		okButton->setEnabled(true);
	else
		okButton->setEnabled(false);
}

int DlgLoadRemoteDeck::getDeckId() const
{
	return dirView->currentItem()->data(1, Qt::DisplayRole).toInt();
}
