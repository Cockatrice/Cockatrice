#ifndef TOURNAMENT_TAB_GAME_EXTENSION_H
#define TOURNAMENT_TAB_GAME_EXTENSION_H

#include <QObject>
#include <QPointer>
#include <libcockatrice/protocol/pb/event_tournament_state.pb.h>

class QLabel;
class QPushButton;
class QWidget;
class TournamentWidget;
class TabGame;

/**
 * @class TournamentTabGameExtension
 * @ingroup GameViews
 * @brief Attaches tournament behavior to a TabGame hosting a tournament hub game.
 *
 * Owns the tournament overview page inside the tab's stacked main widget,
 * mirrors tournament state onto the always-visible deck-view status strip,
 * and routes user intents (open match game, change settings, switch pages)
 * between the two views.
 */
class TournamentTabGameExtension : public QObject
{
    Q_OBJECT
public:
    explicit TournamentTabGameExtension(TabGame *parent);

    void initializeTournamentMode();
    void retranslateUi();

private slots:
    void onTournamentStateChanged(const Event_TournamentState &state);
    void showOverviewPage();
    void showDeckViewPage();
    void openMatchGame(int gameId);

private:
    void connectSignals();
    void updateDeckViewStrip(const Event_TournamentState &state);
    void updateNavigationButtons(const Event_TournamentState &state);

    QPointer<TabGame> tabGame;
    TournamentWidget *tournamentWidget = nullptr;
    QWidget *tournamentOverviewWidget = nullptr;
    QPushButton *backToGameButton = nullptr;
    QPushButton *standingsButton = nullptr;
    QLabel *deckViewStatusLabel = nullptr;
    Event_TournamentState::TournamentPhase lastKnownPhase = Event_TournamentState::PHASE_DECK_BUILDING;
    bool hasLastKnownPhase = false;
};

#endif // TOURNAMENT_TAB_GAME_EXTENSION_H
