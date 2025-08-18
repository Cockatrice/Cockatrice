#ifndef WINDOW_DECKEDITOR_H
#define WINDOW_DECKEDITOR_H

#include "../../game/cards/card_info.h"
#include "../../utility/key_signals.h"
#include "../ui/widgets/visual_deck_storage/deck_preview/deck_preview_deck_tags_display_widget.h"
#include "abstract_tab_deck_editor.h"

class CardDatabaseModel;
class CardDatabaseDisplayModel;
class DeckListModel;

class QLabel;
class DeckLoader;

class TabDeckEditor : public AbstractTabDeckEditor
{
    Q_OBJECT

protected slots:
    void loadLayout() override;
    void restartLayout() override;
    void freeDocksSize() override;
    void refreshShortcuts() override;

    bool eventFilter(QObject *o, QEvent *e) override;
    void dockVisibleTriggered() override;
    void dockFloatingTriggered() override;
    void dockTopLevelChanged(bool topLevel) override;

public:
    explicit TabDeckEditor(TabSupervisor *_tabSupervisor);
    void retranslateUi() override;
    QString getTabText() const override;
    void createMenus() override;

public slots:
    void showPrintingSelector() override;
};

#endif
