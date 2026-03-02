/**
 * @file tab_deck_storage_visual.h
 * @ingroup Tabs
 * @brief TODO: Document this.
 */

#ifndef TAB_DECK_STORAGE_VISUAL_H
#define TAB_DECK_STORAGE_VISUAL_H

#include "../tab.h"

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
    void retranslateUi() override
    {
    }
    [[nodiscard]] QString getTabText() const override
    {
        return tr("Visual Deck Storage");
    }

public slots:
    void actOpenLocalDeck(const QString &filePath);

signals:
    void openDeckEditor(const LoadedDeck &deck);

private:
    VisualDeckStorageWidget *visualDeckStorageWidget;
};

#endif
