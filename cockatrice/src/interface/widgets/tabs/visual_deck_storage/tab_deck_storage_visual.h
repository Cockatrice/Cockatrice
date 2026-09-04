/**
 * @file tab_deck_storage_visual.h
 * @ingroup Tabs
 */
//! \todo Document this file.

#ifndef TAB_DECK_STORAGE_VISUAL_H
#define TAB_DECK_STORAGE_VISUAL_H

#include "../../deck_share/share_bar_widget.h"
#include "../tab.h"

#include <QStringList>
#include <libcockatrice/network/client/abstract/abstract_client.h>

struct LoadedDeck;
class AbstractClient;
class CommandContainer;
class DeckPreviewWidget;
class QFileSystemModel;
class QGroupBox;
class QToolBar;
class QTreeView;
class QTreeWidget;
class QTreeWidgetItem;
class Response;
class VisualDeckStorageWidget;

class TabDeckStorageVisual final : public Tab
{
    Q_OBJECT
public:
    explicit TabDeckStorageVisual(TabSupervisor *_tabSupervisor);
    void retranslateUi() override;

    [[nodiscard]] QString getTabText() const override
    {
        return tr("Visual Deck Storage");
    }

    /**
     * @brief Enters share-selection mode, optionally preselecting the given deck files.
     */
    void enterShareMode(const QStringList &preselectFiles = {});

    /**
     * @brief Leaves share-selection mode and clears the selection.
     */
    void exitShareMode();

    [[nodiscard]] bool isShareModeActive() const
    {
        return shareBar->isVisible();
    }

public slots:
    void actOpenLocalDeck(const QString &filePath);
    void actShareDeck(const QString &filePath);

signals:
    void openDeckEditor(const LoadedDeck &deck);

private slots:
    void actShareSelected();
    void shareFinished(const Response &response, const CommandContainer &commandContainer);
    void onShareSelectionChanged();
    void handleConnectionChanged(ClientStatus status);

private:
    void showShareNotice(const QString &message);
    void updateShareHint();

    VisualDeckStorageWidget *visualDeckStorageWidget;

    ShareBarWidget *shareBar;
    bool shareDeckAvailable = false;
};

#endif