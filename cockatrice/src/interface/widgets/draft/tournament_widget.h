#ifndef TOURNAMENT_WIDGET_H
#define TOURNAMENT_WIDGET_H

#include <QWidget>
#include <libcockatrice/protocol/pb/event_tournament_state.pb.h>

class QLabel;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;

/**
 * @class TournamentWidget
 * @ingroup GameViews
 * @brief Displays live standings and pairings for a tournament game.
 *
 * Updated exclusively via updateTournamentState(); the widget holds no
 * logic of its own beyond mapping protocol state to table rows. Navigation
 * decisions (which pairing belongs to the local player) are resolved here so
 * callers only need to react to openMatchGameRequested().
 */
class TournamentWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TournamentWidget(QWidget *parent = nullptr);

    void updateTournamentState(const Event_TournamentState &state);
    void setLocalPlayerId(int playerId);
    void setIsSpectator(bool spectator);
    void retranslateUi();

signals:
    /*! Emitted when the user asks to open the match game of their current pairing. */
    void openMatchGameRequested(int gameId);

private:
    void rebuildTables(const Event_TournamentState &state);
    void resolveCurrentGameId(const Event_TournamentState &state);
    void updateOpenMatchButton();
    [[nodiscard]] QString getPlayerName(const Event_TournamentState &state, int playerId) const;
    [[nodiscard]] int getGamesPerMatch(const Event_TournamentState &state) const;

    QLabel *statusLabel;
    QLabel *roundLabel;
    QTableWidget *standingsTable;
    QTableWidget *pairingsTable;
    QPushButton *openMatchButton;

    int localPlayerId = -1;
    bool isSpectator = false;
    int currentGameId = -1;
    bool hasOwnLivePairing = false;
    bool hasAnyLivePairing = false;
    Event_TournamentState lastState;
};

#endif // TOURNAMENT_WIDGET_H
