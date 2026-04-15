#ifndef COCKATRICE_DECK_EDITOR_DECK_LIST_HISTORY_MANAGER_WIDGET_H
#define COCKATRICE_DECK_EDITOR_DECK_LIST_HISTORY_MANAGER_WIDGET_H

#ifndef COCKATRICE_DECK_UNDO_WIDGET_H
#define COCKATRICE_DECK_UNDO_WIDGET_H

#include <QWidget>

class DeckStateManager;
class DeckListStyleProxy;
class QListWidgetItem;
class QHBoxLayout;
class QToolButton;
class SettingsButtonWidget;
class QLabel;
class QListWidget;

class DeckListHistoryManagerWidget : public QWidget
{
    Q_OBJECT

signals:
    void requestDisplayWidgetSync();

public slots:
    void retranslateUi();

public:
    explicit DeckListHistoryManagerWidget(DeckStateManager *deckStateManager,
                                          DeckListStyleProxy *styleProxy,
                                          QWidget *parent = nullptr);

private slots:
    void refreshList();
    void onListClicked(const QListWidgetItem *item);
    void doUndo();
    void doRedo();

private:
    DeckStateManager *deckStateManager;
    DeckListStyleProxy *styleProxy;

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
