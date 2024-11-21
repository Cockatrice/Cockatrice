#include "tab_deck_storage_visual.h"

#include "../../../deck/deck_list_model.h"
#include "../../../deck/deck_loader.h"
#include "../../../deck/deck_view.h"
#include "../../../game/cards/card_database_manager.h"
#include "../../../game/cards/card_database_model.h"
#include "../../../main.h"
#include "../../../settings/cache_settings.h"
#include "../../dbconverter/src/mocks.h"
#include "../../ui/picture_loader.h"
#include "../../ui/widgets/cards/card_info_picture_widget.h"
#include "../../ui/widgets/cards/card_info_picture_with_text_overlay_widget.h"
#include "../../ui/widgets/general/layout_containers/flow_widget.h"
#include "../../ui/widgets/general/layout_containers/overlap_widget.h"
#include "pb/command_deck_del.pb.h"

#include <QAction>
#include <QDebug>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QScreen>
#include <QToolBar>
#include <QTreeView>

class FlowLayout;
TabDeckStorageVisual::TabDeckStorageVisual(TabSupervisor *_tabSupervisor, AbstractClient *_client)
    : Tab(_tabSupervisor), client(_client)
{
    deck_list_model = new DeckListModel(this);
    deck_list_model->setObjectName("visualDeckModel");

    flow_widget = new FlowWidget(this, Qt::ScrollBarAlwaysOff,Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    this->setCentralWidget(flow_widget);

    // Start adding our Widgets to our FlowLayout...

    getBannerCardsForDecks();

    leftToolBar = new QToolBar;
    leftToolBar->setOrientation(Qt::Horizontal);
    leftToolBar->setIconSize(QSize(32, 32));
    QHBoxLayout *leftToolBarLayout = new QHBoxLayout;
    leftToolBarLayout->addStretch();
    leftToolBarLayout->addWidget(leftToolBar);
    leftToolBarLayout->addStretch();

    aOpenLocalDeck = new QAction(this);
    aOpenLocalDeck->setIcon(QPixmap("theme:icons/pencil"));
    connect(aOpenLocalDeck, SIGNAL(triggered()), this, SLOT(actOpenLocalDeck()));
    aDeleteLocalDeck = new QAction(this);
    aDeleteLocalDeck->setIcon(QPixmap("theme:icons/remove_row"));
    connect(aDeleteLocalDeck, SIGNAL(triggered()), this, SLOT(actDeleteLocalDeck()));

    leftToolBar->addAction(aOpenLocalDeck);
    leftToolBar->addAction(aDeleteLocalDeck);
    retranslateUi();
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

QStringList TabDeckStorageVisual::getBannerCardsForDecks()
{
    QStringList allFiles;

    // QDirIterator with QDir::Files and QDir::NoSymLinks ensures only files are listed (no directories or symlinks)
    QDirIterator it(SettingsCache::instance().getDeckPath(), QDir::Files | QDir::NoSymLinks,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        allFiles << it.next(); // Add each file path to the list
    }

    auto deck_loader = new DeckLoader();
    foreach (const QString &file, allFiles) {
        qDebug() << file;
        deck_loader->loadFromFile(file, DeckLoader::CockatriceFormat);
        deck_list_model->setDeckList(new DeckLoader(*deck_loader));

        InnerDecklistNode *listRoot = deck_list_model->getDeckList()->getRoot();

        bool shouldExitOuterLoop = false; // Flag to indicate if we should exit the outer loop
        for (int i = 0; i < listRoot->size(); i++) {
            InnerDecklistNode *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));

            for (int j = 0; j < currentZone->size(); j++) {
                DecklistCardNode *currentCard = dynamic_cast<DecklistCardNode *>(currentZone->at(j));
                if (!currentCard)
                    continue;

                for (int k = 0; k < currentCard->getNumber(); ++k) {
                    CardInfoPtr info = CardDatabaseManager::getInstance()->getCard(currentCard->getName());
                    if (info) {
                        if (info->getCardType().contains("Legendary Creature")) {
                            //qDebug() << currentCard->getName();
                            //qDebug() << info->getCmc();
                            //qDebug() << info->getProperties();
                            //qDebug() << info->getCardType();

                            CardInfoPerSetMap setMap = info->getSets();
                            OverlapWidget *printings_group_widget =
                                new OverlapWidget(this, 80, 0, 0, Qt::Orientation::Vertical);
                            for (auto set : setMap) {
                                // qDebug() << set.getPtr()->getLongName();
                                CardInfoPtr set_info = CardDatabaseManager::getInstance()->getCardByNameAndProviderId(
                                    currentCard->getName(), set.getProperty("uuid"));
                                CardInfoPictureWithTextOverlayWidget * display = new CardInfoPictureWithTextOverlayWidget(printings_group_widget, true);
                                display->setCard(set_info);
                                display->setOverlayText(deck_loader->getName());
                                display->setFontSize(24);
                                printings_group_widget->addWidget(display);
                            }
                            flow_widget->addWidget(printings_group_widget);

                            //db->getPreferredPrintingForCard(info->getName());

                            //PictureLoader::getCardBackPixmap();

                            shouldExitOuterLoop = true; // Set the flag to exit the outer loop
                            break; // Break the innermost loop
                        }
                    } else {
                        //qDebug() << "Card not found in database!";
                    }

                    // emit newCardAdded(newCard);
                }
                if (shouldExitOuterLoop) {
                    break; // Break the middle loop if flag is set
                }
            }
            if (shouldExitOuterLoop) {
                break; // Break the outer loop if flag is set
            }
        }
    }

    return QStringList("lol");
}

void TabDeckStorageVisual::actOpenLocalDeck()
{
    QModelIndex curLeft = localDirView->selectionModel()->currentIndex();
    if (localDirModel->isDir(curLeft))
        return;
    QString filePath = localDirModel->filePath(curLeft);

    DeckLoader deckLoader;
    if (!deckLoader.loadFromFile(filePath, DeckLoader::CockatriceFormat))
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

    // You can add any additional logic you need here.
}
