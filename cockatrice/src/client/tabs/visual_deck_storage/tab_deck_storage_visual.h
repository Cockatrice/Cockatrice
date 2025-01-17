#ifndef TAB_DECK_STORAGE_VISUAL_H
#define TAB_DECK_STORAGE_VISUAL_H

#include "../tab.h"

class AbstractClient;
class CommandContainer;
class DeckLoader;
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
    void retranslateUi() override{};
    [[nodiscard]] QString getTabText() const override
    {
        return tr("Visual Deck storage");
    }

public slots:
    void actOpenLocalDeck(QMouseEvent * /*event*/, DeckPreviewWidget *instance);

signals:
    void openDeckEditor(const DeckLoader *deckLoader);

private:
    VisualDeckStorageWidget *visualDeckStorageWidget;
};

#endif
