#include "tournament_widget.h"

#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace
{
const int PLAYER_COLUMN = 0;
const int DECK_COLUMN = 1;
const int WINS_COLUMN = 2;
const int LOSSES_COLUMN = 3;
const int DRAWS_COLUMN = 4;

const int PLAYER1_COLUMN = 0;
const int SCORE_COLUMN = 1;
const int STATUS_COLUMN = 2;
const int PLAYER2_COLUMN = 3;
} // namespace

TournamentWidget::TournamentWidget(QWidget *parent) : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);

    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(statusLabel);

    roundLabel = new QLabel(this);
    mainLayout->addWidget(roundLabel);

    standingsTable = new QTableWidget(this);
    standingsTable->setColumnCount(5);
    standingsTable->horizontalHeader()->setStretchLastSection(true);
    standingsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    standingsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    standingsTable->verticalHeader()->setVisible(false);
    mainLayout->addWidget(standingsTable);

    pairingsTable = new QTableWidget(this);
    pairingsTable->setColumnCount(4);
    pairingsTable->horizontalHeader()->setStretchLastSection(true);
    pairingsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    pairingsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    pairingsTable->verticalHeader()->setVisible(false);
    mainLayout->addWidget(pairingsTable);

    openMatchButton = new QPushButton(this);
    openMatchButton->setEnabled(false);
    connect(openMatchButton, &QPushButton::clicked, this, [this]() { emit openMatchGameRequested(currentGameId); });
    mainLayout->addWidget(openMatchButton);

    retranslateUi();
}

void TournamentWidget::retranslateUi()
{
    standingsTable->setHorizontalHeaderLabels({tr("Player"), tr("Deck"), tr("W"), tr("L"), tr("D")});
    pairingsTable->setHorizontalHeaderLabels({tr("Player 1"), tr("Score"), tr("Status"), tr("Player 2")});
    openMatchButton->setText(tr("Open match game"));

    if (lastState.has_phase()) {
        updateTournamentState(lastState);
    } else {
        statusLabel->setText(tr("Tournament"));
        roundLabel->setVisible(true);
        roundLabel->setText(tr("Waiting for the tournament to start"));
    }
}

void TournamentWidget::setLocalPlayerId(int playerId)
{
    localPlayerId = playerId;
}

QString TournamentWidget::getPlayerName(const Event_TournamentState &state, int playerId) const
{
    for (int i = 0; i < state.players_size(); ++i) {
        if (state.players(i).player_id() == playerId) {
            return QString::fromStdString(state.players(i).player_name());
        }
    }
    return tr("Player %1").arg(playerId);
}

void TournamentWidget::updateTournamentState(const Event_TournamentState &state)
{
    lastState = state;

    int gamesPerMatch = getGamesPerMatch(state);
    QString statusText;
    switch (state.phase()) {
        case Event_TournamentState::PHASE_DECK_BUILDING:
            statusText = gamesPerMatch > 1 ? tr("Tournament - Deck building (Best of %1)").arg(gamesPerMatch * 2 - 1)
                                           : tr("Tournament - Deck building");
            break;
        case Event_TournamentState::PHASE_PLAYING:
            statusText = gamesPerMatch > 1 ? tr("Tournament - Playing (Best of %1)").arg(gamesPerMatch * 2 - 1)
                                           : tr("Tournament - Playing");
            break;
        case Event_TournamentState::PHASE_FINISHED:
            statusText = gamesPerMatch > 1 ? tr("Tournament - Finished (Best of %1)").arg(gamesPerMatch * 2 - 1)
                                           : tr("Tournament - Finished");
            break;
        default:
            statusText = tr("Tournament");
            break;
    }
    statusLabel->setText(statusText);

    // The server does not report rounds before pairing starts; avoid showing "Round 0 of 0".
    bool roundStarted = state.current_round() > 0 || state.total_rounds() > 0;
    roundLabel->setVisible(roundStarted);
    if (roundStarted) {
        roundLabel->setText(tr("Round %1 of %2").arg(state.current_round()).arg(state.total_rounds()));
    }

    rebuildTables(state);
    resolveCurrentGameId(state);
    updateOpenMatchButton();
}

void TournamentWidget::updateOpenMatchButton()
{
    bool playingPhase = lastState.phase() == Event_TournamentState::PHASE_PLAYING;
    openMatchButton->setVisible(playingPhase);
    if (!playingPhase) {
        return;
    }

    if (hasOwnLivePairing) {
        openMatchButton->setEnabled(true);
        openMatchButton->setText(tr("Open match game"));
        openMatchButton->setToolTip(tr("Switch to your current match game"));
    } else if (localPlayerId == -1 && hasAnyLivePairing) {
        // Spectators have no pairing of their own but may watch any running match.
        openMatchButton->setEnabled(true);
        openMatchButton->setText(tr("Spectate live match"));
        openMatchButton->setToolTip(tr("Watch a match that is currently running"));
    } else {
        openMatchButton->setEnabled(false);
        openMatchButton->setText(tr("Open match game"));
        openMatchButton->setToolTip(tr("No match game is running for you right now"));
    }
}

void TournamentWidget::rebuildTables(const Event_TournamentState &state)
{
    standingsTable->setRowCount(state.players_size());
    for (int i = 0; i < state.players_size(); ++i) {
        const auto &player = state.players(i);

        auto *nameItem = new QTableWidgetItem(QString::fromStdString(player.player_name()));
        nameItem->setToolTip(QString::fromStdString(player.player_name()));
        standingsTable->setItem(i, PLAYER_COLUMN, nameItem);

        auto *deckItem = new QTableWidgetItem(player.deck_submitted() ? tr("Submitted") : tr("Pending"));
        deckItem->setToolTip(player.deck_submitted() ? tr("Deck submitted") : tr("Still choosing a deck"));
        standingsTable->setItem(i, DECK_COLUMN, deckItem);

        standingsTable->setItem(i, WINS_COLUMN, new QTableWidgetItem(QString::number(player.wins())));
        standingsTable->setItem(i, LOSSES_COLUMN, new QTableWidgetItem(QString::number(player.losses())));
        standingsTable->setItem(i, DRAWS_COLUMN, new QTableWidgetItem(QString::number(player.draws())));
    }

    pairingsTable->setRowCount(state.pairings_size());
    for (int i = 0; i < state.pairings_size(); ++i) {
        const auto &pairing = state.pairings(i);

        QString name1 = getPlayerName(state, pairing.player1_id());
        QString name2 = pairing.player2_id() == -1 ? tr("BYE") : getPlayerName(state, pairing.player2_id());

        QString scoreStr;
        if (getGamesPerMatch(state) > 1 && pairing.player2_id() != -1) {
            scoreStr = tr("%1 - %2").arg(pairing.player1_match_wins()).arg(pairing.player2_match_wins());
        }

        QString statusStr;
        if (pairing.player2_id() == -1) {
            statusStr = tr("BYE");
        } else if (pairing.winner_id() != -1) {
            statusStr = tr("Finished");
        } else {
            statusStr = tr("vs");
        }

        pairingsTable->setItem(i, PLAYER1_COLUMN, new QTableWidgetItem(name1));
        pairingsTable->setItem(i, SCORE_COLUMN, new QTableWidgetItem(scoreStr));
        pairingsTable->setItem(i, STATUS_COLUMN, new QTableWidgetItem(statusStr));
        pairingsTable->setItem(i, PLAYER2_COLUMN, new QTableWidgetItem(name2));
    }
}

int TournamentWidget::getGamesPerMatch(const Event_TournamentState &state) const
{
    return state.has_settings() ? state.settings().games_per_match() : 1;
}

void TournamentWidget::resolveCurrentGameId(const Event_TournamentState &state)
{
    currentGameId = -1;
    hasOwnLivePairing = false;
    hasAnyLivePairing = false;

    for (int i = 0; i < state.pairings_size(); ++i) {
        const auto &pairing = state.pairings(i);
        if (pairing.game_id() == -1) {
            continue;
        }
        if (!hasOwnLivePairing && localPlayerId != -1 &&
            (pairing.player1_id() == localPlayerId || pairing.player2_id() == localPlayerId)) {
            currentGameId = pairing.game_id();
            hasOwnLivePairing = true;
        }
        if (!hasAnyLivePairing) {
            hasAnyLivePairing = true;
            if (!hasOwnLivePairing) {
                currentGameId = pairing.game_id();
            }
        }
        if (hasOwnLivePairing && hasAnyLivePairing) {
            return;
        }
    }
}
