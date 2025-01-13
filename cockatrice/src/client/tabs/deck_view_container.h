#ifndef DECK_VIEW_CONTAINER_H
#define DECK_VIEW_CONTAINER_H

#include "../../client/ui/widgets/cards/card_info_frame_widget.h"
#include "../../deck/deck_loader.h"
#include "pb/event_leave.pb.h"
#include "tab.h"

#include <QCompleter>
#include <QPushButton>
#include <pb/response.pb.h>

class VisualDeckStorageWidget;
class DeckPreviewWidget;
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
    void replaceDeckStorageWithDeckView(QMouseEvent *event, DeckPreviewWidget *instance);
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
    void setButtonsVisible(bool _visible);
    void setReadyStart(bool ready);
    void readyAndUpdate();
    void setSideboardLocked(bool locked);
    void setDeck(const DeckLoader &deck);
    void loadDeckFromFile(const QString &filePath);
};

#endif // DECK_VIEW_CONTAINER_H
