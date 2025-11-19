#ifndef COCKATRICE_DECK_EDITOR_DECK_LIST_HISTORY_MANAGER_WIDGET_H
#define COCKATRICE_DECK_EDITOR_DECK_LIST_HISTORY_MANAGER_WIDGET_H

#ifndef COCKATRICE_DECK_UNDO_WIDGET_H
#define COCKATRICE_DECK_UNDO_WIDGET_H

#include "../quick_settings/settings_button_widget.h"
#include "deck_list_style_proxy.h"

#include <QAction>
#include <QHBoxLayout>
#include <QListWidget>
#include <QToolButton>
#include <QWidget>
#include <libcockatrice/deck_list/deck_list_history_manager.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>

class DeckListHistoryManagerWidget : public QWidget
{
    Q_OBJECT

signals:
    void requestDisplayWidgetSync();

public slots:
    void retranslateUi();

public:
    explicit DeckListHistoryManagerWidget(DeckListModel *deckListModel,
                                          DeckListStyleProxy *styleProxy,
                                          DeckListHistoryManager *manager,
                                          QWidget *parent = nullptr);
    void setDeckListModel(DeckListModel *_deckListModel);

private slots:
    void refreshList();
    void onListClicked(QListWidgetItem *item);
    void doUndo();
    void doRedo();

private:
    DeckListModel *deckListModel;
    DeckListStyleProxy *styleProxy;
    DeckListHistoryManager *historyManager;

    QHBoxLayout *layout;
    QAction *aUndo;
    QToolButton *undoButton;
    QAction *aRedo;
    QToolButton *redoButton;
    SettingsButtonWidget *historyButton;
    QLabel *historyLabel;
    QListWidget *historyList;
};

#endif // COCKATRICE_DECK_UNDO_WIDGET_H

#endif // COCKATRICE_DECK_EDITOR_DECK_LIST_HISTORY_MANAGER_WIDGET_H
