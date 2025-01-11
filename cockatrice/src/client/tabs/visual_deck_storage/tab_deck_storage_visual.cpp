#include "tab_deck_storage_visual.h"

#include "../../../game/cards/card_database_model.h"
#include "../tab_supervisor.h"
#include "pb/command_deck_del.pb.h"

#include <QAction>
#include <QDebug>
#include <QDirIterator>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QScreen>
#include <QToolBar>
#include <QTreeView>

class FlowLayout;
TabDeckStorageVisual::TabDeckStorageVisual(TabSupervisor *_tabSupervisor, AbstractClient *_client)
    : Tab(_tabSupervisor), client(_client)
{
    deck_list_model = new DeckListModel(this);
    deck_list_model->setObjectName("visualDeckModel");

    QWidget *container = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(container);
    container->setLayout(layout);
    this->setCentralWidget(container);

    leftToolBar = new QToolBar;
    leftToolBar->setOrientation(Qt::Horizontal);
    leftToolBar->setIconSize(QSize(32, 32));
    QHBoxLayout *leftToolBarLayout = new QHBoxLayout(this);
    leftToolBarLayout->addStretch();
    leftToolBarLayout->addWidget(leftToolBar);
    leftToolBarLayout->addStretch();

    aOpenLocalDeck = new QAction(this);
    aOpenLocalDeck->setIcon(QPixmap("theme:icons/pencil"));
    connect(aOpenLocalDeck, SIGNAL(triggered()), this, SLOT(actOpenLocalDeck()));
    aDeleteLocalDeck = new QAction(this);
    aDeleteLocalDeck->setIcon(QPixmap("theme:icons/remove_row"));
    connect(aDeleteLocalDeck, SIGNAL(triggered()), this, SLOT(actDeleteLocalDeck()));

    connect(this, &TabDeckStorageVisual::openDeckEditor, tabSupervisor, &TabSupervisor::addDeckEditorTab);

    leftToolBar->addAction(aOpenLocalDeck);
    leftToolBar->addAction(aDeleteLocalDeck);

    visualDeckStorageWidget = new VisualDeckStorageWidget(this);
    connect(visualDeckStorageWidget, &VisualDeckStorageWidget::imageDoubleClicked, this,
            &TabDeckStorageVisual::actOpenLocalDeck);

    // layout->addWidget(leftToolBar);
    layout->addWidget(visualDeckStorageWidget);

    retranslateUi();
}

void TabDeckStorageVisual::closeRequest(bool forced)
{
    this->close();
}

void TabDeckStorageVisual::retranslateUi()
{
    aOpenLocalDeck->setText(tr("Open in deck editor"));
    aDeleteLocalDeck->setText(tr("Delete"));
}

QString TabDeckStorageVisual::getTargetPath() const
{
    return {};
}

void TabDeckStorageVisual::actOpenLocalDeck(QMouseEvent *event, DeckPreviewCardPictureWidget *instance)
{
    (void)event;
    DeckLoader deckLoader;
    if (!deckLoader.loadFromFile(instance->filePath, DeckLoader::CockatriceFormat))
        return;

    emit openDeckEditor(&deckLoader);
}

void TabDeckStorageVisual::actDeleteLocalDeck()
{
    QModelIndex curLeft = localDirView->selectionModel()->currentIndex();
    if (localDirModel->isDir(curLeft))
        return;

    if (QMessageBox::warning(this, tr("Delete local file"),
                             tr("Are you sure you want to delete \"%1\"?").arg(localDirModel->fileName(curLeft)),
                             QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    localDirModel->remove(curLeft);
}

void TabDeckStorageVisual::cardUpdateFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Card update process finished with exit code:" << exitCode << "and exit status:" << exitStatus;
}
