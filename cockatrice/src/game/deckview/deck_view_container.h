/**
 * @file deck_view_container.h
 * @ingroup Lobby
 * @brief TODO: Document this.
 */

#ifndef DECK_VIEW_CONTAINER_H
#define DECK_VIEW_CONTAINER_H

#include "../../interface/deck_loader/deck_loader.h"

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
    [[nodiscard]] bool getState() const
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
    QPushButton *loadLocalButton, *loadRemoteButton, *loadFromClipboardButton, *loadFromWebsiteButton;
    QPushButton *unloadDeckButton, *forceStartGameButton;
    ToggleButton *readyStartButton, *sideboardLockButton;
    DeckView *deckView;
    VisualDeckStorageWidget *visualDeckStorageWidget;
    TabGame *parentGame;
    int playerId;

    void tryCreateVisualDeckStorageWidget();
    void sendReadyStartCommand(bool ready);
private slots:
    void switchToDeckSelectView();
    void switchToDeckLoadedView();
    void loadLocalDeck();
    void loadRemoteDeck();
    void loadFromClipboard();
    void loadFromWebsite();
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
    void setDeck(const DeckList &deck);
    void setVisualDeckStorageExists(bool exists);

public slots:
    void loadDeckFromFile(const QString &filePath);
    void loadDeckFromDeckList(const DeckList &deck);
};

#endif // DECK_VIEW_CONTAINER_H
