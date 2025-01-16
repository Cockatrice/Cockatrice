#ifndef DECK_VIEW_CONTAINER_H
#define DECK_VIEW_CONTAINER_H

#include "../../deck/deck_loader.h"

#include <QPushButton>

class QVBoxLayout;
class AbstractCardItem;
class VisualDeckStorageWidget;
class DeckPreviewWidget;
class Response;
class TabGame;
class DeckView;

/**
 * Custom QButton implementation in order to have the red/green toggling square around the button
 */
class ToggleButton : public QPushButton
{
    Q_OBJECT
private:
    bool state;
signals:
    void stateChanged();

public:
    explicit ToggleButton(QWidget *parent = nullptr);
    bool getState() const
    {
        return state;
    }
    void setState(bool _state);

protected:
    void paintEvent(QPaintEvent *event) override;
};

/**
 * This widget contains the deck selection view that is used before a game begins.
 */
class DeckViewContainer : public QWidget
{
    Q_OBJECT
private:
    QVBoxLayout *deckViewLayout;
    QPushButton *loadLocalButton, *loadRemoteButton, *unloadDeckButton, *forceStartGameButton;
    ToggleButton *readyStartButton, *sideboardLockButton;
    DeckView *deckView;
    VisualDeckStorageWidget *visualDeckStorageWidget;
    TabGame *parentGame;
    int playerId;
private slots:
    void switchToDeckSelectView();
    void switchToDeckLoadedView();
    void loadVisualDeck(QMouseEvent *event, DeckPreviewWidget *instance);
    void loadLocalDeck();
    void loadRemoteDeck();
    void unloadDeck();
    void readyStart();
    void forceStart();
    void deckSelectFinished(const Response &r);
    void sideboardPlanChanged();
    void sideboardLockButtonClicked();
    void updateSideboardLockButtonText();
    void refreshShortcuts();
signals:
    void newCardAdded(AbstractCardItem *card);
    void notIdle();

public:
    DeckViewContainer(int _playerId, TabGame *parent);
    void retranslateUi();
    void setReadyStart(bool ready);
    void readyAndUpdate();
    void setSideboardLocked(bool locked);
    void setDeck(const DeckLoader &deck);
    void loadDeckFromFile(const QString &filePath);
};

#endif // DECK_VIEW_CONTAINER_H
