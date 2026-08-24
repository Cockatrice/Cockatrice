#include "tournament_tab_game_extension.h"

#include "../../../game/game_event_handler.h"
#include "../../../game/player/player_logic.h"
#include "../../widgets/dialogs/dlg_tournament_settings.h"
#include "../../widgets/draft/tournament_widget.h"
#include "tab_game.h"
#include "tab_supervisor.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <libcockatrice/protocol/pb/command_tournament.pb.h>
#include <libcockatrice/protocol/pb/event_tournament_state.pb.h>

TournamentTabGameExtension::TournamentTabGameExtension(TabGame *parent) : QObject(parent), tabGame(parent)
{
    tournamentOverviewWidget = new QWidget(parent);
    auto *overviewLayout = new QVBoxLayout(tournamentOverviewWidget);

    auto *headerLayout = new QHBoxLayout;
    backToGameButton = new QPushButton(tournamentOverviewWidget);
    connect(backToGameButton, &QPushButton::clicked, this, &TournamentTabGameExtension::showDeckViewPage);
    headerLayout->addWidget(backToGameButton);
    headerLayout->addStretch();
    overviewLayout->addLayout(headerLayout);

    tournamentWidget = new TournamentWidget(tournamentOverviewWidget);
    tournamentWidget->setLocalPlayerId(parent->getGame()->getPlayerManager()->getLocalPlayerId());
    overviewLayout->addWidget(tournamentWidget);

    parent->getMainWidget()->addWidget(tournamentOverviewWidget);

    deckViewStatusLabel = new QLabel(parent->getDeckViewContainerWidget());
    deckViewStatusLabel->setStyleSheet("color: palette(placeholderText);");
    deckViewStatusLabel->setVisible(false);

    standingsButton = new QPushButton(parent->getDeckViewContainerWidget());
    connect(standingsButton, &QPushButton::clicked, this, &TournamentTabGameExtension::showOverviewPage);
    standingsButton->setVisible(false);

    connectSignals();
}

bool TournamentTabGameExtension::isLocalPlayerHost() const
{
    return tabGame->getGame()->getPlayerManager()->getLocalPlayerId() ==
           tabGame->getGame()->getGameState()->getHostId();
}

void TournamentTabGameExtension::connectSignals()
{
    auto *handler = tabGame->getGame()->getGameEventHandler();
    connect(handler, &GameEventHandler::tournamentStateChanged, this,
            &TournamentTabGameExtension::onTournamentStateChanged);
    connect(tournamentWidget, &TournamentWidget::openMatchGameRequested, this,
            &TournamentTabGameExtension::openMatchGame);
}

void TournamentTabGameExtension::initializeTournamentMode()
{
    if (!tabGame) {
        return;
    }

    auto *deckLayout = tabGame->getDeckViewContainerLayout();
    int index = 0;
    deckLayout->insertWidget(index++, deckViewStatusLabel);
    if (isLocalPlayerHost()) {
        settingsButton = new QPushButton(tabGame->getDeckViewContainerWidget());
        connect(settingsButton, &QPushButton::clicked, this, &TournamentTabGameExtension::showTournamentSettingsDialog);
        deckLayout->insertWidget(index++, settingsButton);
    }
    deckLayout->insertWidget(index++, standingsButton);
    deckLayout->insertSpacing(index, 4);
}

void TournamentTabGameExtension::retranslateUi()
{
    backToGameButton->setText(tr("Back to game view"));
    standingsButton->setText(tr("Tournament standings"));
    if (settingsButton) {
        settingsButton->setText(tr("Tournament Settings"));
    }
    tournamentWidget->retranslateUi();
}

void TournamentTabGameExtension::updateDeckViewStrip(const Event_TournamentState &state)
{
    int submitted = 0;
    for (int i = 0; i < state.players_size(); ++i) {
        if (state.players(i).deck_submitted()) {
            ++submitted;
        }
    }

    QString phaseText;
    switch (state.phase()) {
        case Event_TournamentState::PHASE_DECK_BUILDING:
            phaseText = tr("Deck building");
            break;
        case Event_TournamentState::PHASE_PLAYING:
            phaseText = tr("Round in progress");
            break;
        case Event_TournamentState::PHASE_FINISHED:
            phaseText = tr("Finished");
            break;
        default:
            phaseText = tr("Unknown phase");
            break;
    }

    // The submission count only matters while players are still picking decks.
    QString text =
        state.phase() == Event_TournamentState::PHASE_DECK_BUILDING
            ? tr("Tournament: %1 - %2/%3 decks submitted").arg(phaseText).arg(submitted).arg(state.players_size())
            : tr("Tournament: %1").arg(phaseText);
    deckViewStatusLabel->setText(text);
    deckViewStatusLabel->setVisible(true);
}

void TournamentTabGameExtension::updateNavigationButtons(const Event_TournamentState &state)
{
    bool showStandings =
        state.phase() == Event_TournamentState::PHASE_PLAYING || state.phase() == Event_TournamentState::PHASE_FINISHED;
    standingsButton->setVisible(showStandings);
    if (settingsButton) {
        settingsButton->setVisible(state.phase() == Event_TournamentState::PHASE_DECK_BUILDING);
    }
}

void TournamentTabGameExtension::onTournamentStateChanged(const Event_TournamentState &state)
{
    if (!tabGame) {
        return;
    }

    updateDeckViewStrip(state);
    updateNavigationButtons(state);
    tournamentWidget->updateTournamentState(state);

    // Switch to the standings page only when the phase itself changes, so score
    // updates never yank the user away while they are looking at their deck.
    // Instant switches are the app-wide baseline today; motion and sound cues
    // for this transition are deferred to the Game dressing phase.
    bool overviewPhase =
        state.phase() == Event_TournamentState::PHASE_PLAYING || state.phase() == Event_TournamentState::PHASE_FINISHED;
    if (overviewPhase && (!hasLastKnownPhase || state.phase() != lastKnownPhase)) {
        tabGame->getMainWidget()->setCurrentWidget(tournamentOverviewWidget);
    }
    lastKnownPhase = state.phase();
    hasLastKnownPhase = true;
}

void TournamentTabGameExtension::showOverviewPage()
{
    if (tabGame) {
        tabGame->getMainWidget()->setCurrentWidget(tournamentOverviewWidget);
    }
}

void TournamentTabGameExtension::showDeckViewPage()
{
    if (tabGame) {
        tabGame->getMainWidget()->setCurrentWidget(tabGame->getDeckViewContainerWidget());
    }
}

void TournamentTabGameExtension::showTournamentSettingsDialog()
{
    DlgTournamentSettings dlg(tabGame);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    DlgTournamentSettingsResult result = dlg.getResult();

    PlayerLogic *localPlayer = tabGame->getGame()->getPlayerManager()->getActiveLocalPlayer(-1);
    if (!localPlayer) {
        TabSupervisor::actShowPopup(tr("You are not an active player in this game."));
        return;
    }

    Command_TournamentSettingsSelect cmd;
    cmd.mutable_settings()->set_games_per_match(result.gamesPerMatch);
    tabGame->getGame()->getGameEventHandler()->sendGameCommand(cmd, localPlayer->getPlayerInfo()->getId());
}

void TournamentTabGameExtension::openMatchGame(int gameId)
{
    if (!tabGame || gameId <= 0) {
        return;
    }

    // Players are auto-joined into their match game by the server, so the tab
    // usually already exists and switching is enough. Otherwise (e.g. after
    // leaving the match) fall back to joining as spectator.
    if (tabGame->switchToGameTab(gameId)) {
        return;
    }

    int roomId = tabGame->getGame()->getGameMetaInfo()->proto().room_id();
    tabGame->getTabSupervisor()->spectatorJoinGame(gameId, roomId);
}
