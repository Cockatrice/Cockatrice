#ifndef TAB_DECK_STORAGE_VISUAL_H
#define TAB_DECK_STORAGE_VISUAL_H

#include "../../../deck/deck_list_model.h"
#include "../../../deck/deck_view.h"
#include "../../ui/layouts/flow_layout.h"
#include "../../ui/layouts/overlap_layout.h"
#include "../../ui/widgets/cards/card_info_picture_with_text_overlay_widget.h"
#include "../../ui/widgets/cards/deck_preview_card_picture_widget.h"
#include "../../ui/widgets/general/layout_containers/flow_widget.h"
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

class TabDeckStorageVisual : public Tab
{
    Q_OBJECT
private:
    AbstractClient *client;
    QTreeView *localDirView;
    QFileSystemModel *localDirModel;
    QToolBar *leftToolBar;
    QGroupBox *leftGroupBox;
    FlowWidget *flow_widget;
    DeckListModel *deck_list_model;
    QMap<QString, DeckViewCardContainer *> cardContainers;

    QAction *aOpenLocalDeck, *aDeleteLocalDeck;
    QString getTargetPath() const;
    QStringList getBannerCardsForDecks();
private slots:
    void actOpenLocalDeck(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);
    void actDeleteLocalDeck();

public:
    TabDeckStorageVisual(TabSupervisor *_tabSupervisor, AbstractClient *_client);
    void retranslateUi();
    QString getTabText() const
    {
        return tr("Deck storage");
    }
public slots:
    void cardUpdateFinished(int exitCode, QProcess::ExitStatus exitStatus);
signals:
    void openDeckEditor(const DeckLoader *deckLoader);
};

#endif
