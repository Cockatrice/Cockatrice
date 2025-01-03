#ifndef TAB_DECK_STORAGE_VISUAL_H
#define TAB_DECK_STORAGE_VISUAL_H

#include "../../../deck/deck_list_model.h"
#include "../../../deck/deck_view.h"
#include "../../ui/widgets/cards/deck_preview_card_picture_widget.h"
#include "../../ui/widgets/visual_deck_storage/visual_deck_storage_widget.h"
#include "../tab.h"

#include <QProcess>

class AbstractClient;
class QTreeView;
class QFileSystemModel;
class QToolBar;
class QTreeWidget;
class QTreeWidgetItem;
class QGroupBox;
class CommandContainer;
class Response;
class DeckLoader;

class TabDeckStorageVisual final : public Tab
{
    Q_OBJECT
public:
    TabDeckStorageVisual(TabSupervisor *_tabSupervisor, AbstractClient *_client);

    void retranslateUi() override;
    QString getTabText() const override
    {
        return tr("Visual Deck storage");
    }
public slots:
    void cardUpdateFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void closeRequest() override;
signals:
    void openDeckEditor(const DeckLoader *deckLoader);

private:
    QWidget *container;
    QHBoxLayout *layout;
    AbstractClient *client;
    QTreeView *localDirView;
    QFileSystemModel *localDirModel;
    QToolBar *leftToolBar;
    QGroupBox *leftGroupBox;
    VisualDeckStorageWidget *visualDeckStorageWidget;
    DeckListModel *deck_list_model;
    QAction *aOpenLocalDeck, *aDeleteLocalDeck;
    QString getTargetPath() const;
private slots:
    void actOpenLocalDeck(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);
    void actDeleteLocalDeck();
};

#endif
