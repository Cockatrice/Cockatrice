#include "load_remote_deck.h"

#include "../main.h"
#include "../server/remote/decklist_tree_widget.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

LoadRemoteDeck::LoadRemoteDeck(AbstractClient *_client, QWidget *parent) : QDialog(parent), client(_client)
{
    dirView = new RemoteDeckList_TreeWidget(client);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(dirView);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    setWindowTitle(tr("Load deck"));
    setMinimumWidth(sizeHint().width());
    resize(400, 600);

    connect(dirView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this,
            SLOT(currentItemChanged(const QModelIndex &, const QModelIndex &)));
}

void LoadRemoteDeck::currentItemChanged(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    buttonBox->button(QDialogButtonBox::Ok)
        ->setEnabled(dynamic_cast<RemoteDeckList_TreeModel::FileNode *>(dirView->getNode(current)));
}

int LoadRemoteDeck::getDeckId() const
{
    return dynamic_cast<RemoteDeckList_TreeModel::FileNode *>(
               dirView->getNode(dirView->selectionModel()->currentIndex()))
        ->getId();
}
