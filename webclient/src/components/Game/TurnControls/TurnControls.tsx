import { useMemo, useState } from 'react';
import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';

import { LoadingState, useCurrentGame, useSettings, useWebClient } from '@app/hooks';
import { GameSelectors, useAppSelector } from '@app/store';

import './TurnControls.css';

export interface TurnControlsProps {
  gameId: number | undefined;
  onRequestRollDie: () => void;
  onRequestConcede: () => void;
  onRequestUnconcede: () => void;
  onRequestGameInfo: () => void;
  onToggleRotate90: () => void;
  isRotated: boolean;
}

function TurnControls({
  gameId,
  onRequestRollDie,
  onRequestConcede,
  onRequestUnconcede,
  onRequestGameInfo,
  onToggleRotate90,
  isRotated,
}: TurnControlsProps) {
  const webClient = useWebClient();
  const { game, localPlayer, isSpectator, isJudge, isHost, isStarted } = useCurrentGame(gameId);
  const { status: settingsStatus, value: settings, update: updateSettings } = useSettings();
  const invertVerticalCoordinate = settings?.invertVerticalCoordinate ?? false;

  // Post-kick: the reducer has deleted the game from state but the dialog
  // may still be mounted for a frame while `useGameLifecycle` navigates to
  // /server. Every handler double-checks `game` so a trailing click can't
  // fire a command against a game the server no longer has.
  const hasLiveGame = gameId != null && game != null;

  const [kickAnchor, setKickAnchor] = useState<HTMLElement | null>(null);

  const opponents = useMemo(() => {
    if (!game) {
      return [];
    }
    return Object.values(game.players)
      .filter((p) => p.properties.playerId !== game.localPlayerId)
      .map((p) => ({
        playerId: p.properties.playerId,
        name: p.properties.userInfo?.name ?? `p${p.properties.playerId}`,
      }));
  }, [game]);

  // Local arrows belong to `localPlayerId`; Remove Local Arrows iterates
  // and deletes each one. Matches desktop's Player::actRemoveLocalArrows.
  const localArrows = useAppSelector((state) =>
    gameId != null && game != null
      ? GameSelectors.getArrows(state, gameId, game.localPlayerId)
      : undefined,
  );
  const localArrowIds = useMemo(
    () => (localArrows ? Object.keys(localArrows).map(Number) : []),
    [localArrows],
  );

  // Players (judge or not) act as participants; pure spectators don't.
  // Matches desktop: aConcede/aNextTurn are disabled when isSpectator() without
  // judge privileges (see tab_game.cpp concede enablement + player_menu.cpp
  // getLocalOrJudge gates).
  const isParticipant = gameId != null && game != null && !isSpectator;
  const isConceded = localPlayer?.properties.conceded ?? false;
  const canAdvance =
    gameId != null && game != null && isStarted &&
    (isJudge || game.activePlayerId === game.localPlayerId);
  const canLeave = gameId != null && game != null;
  const canConcede = isParticipant && !isConceded;
  const canUnconcede = isParticipant && isConceded;
  // Rolling dice is a player action; judges may also roll. Pure spectators
  // cannot (desktop exposes it through the player menu, which spectators
  // don't receive).
  const canRoll = gameId != null && (isParticipant || isJudge);
  const canKick = gameId != null && isHost && opponents.length > 0;
  const canRemoveArrows = hasLiveGame && localArrowIds.length > 0;

  const handlePassTurn = () => {
    if (!canAdvance || !hasLiveGame) {
      return;
    }
    webClient.request.game.nextTurn(gameId);
  };

  const handleReverseTurn = () => {
    if (!canAdvance || !hasLiveGame) {
      return;
    }
    webClient.request.game.reverseTurn(gameId);
  };

  const handleNextPhase = () => {
    if (!canAdvance || !hasLiveGame) {
      return;
    }
    // Desktop wraps at 11 → 0 (the Phase enum is 0–10). When no phase is
    // active yet (activePhase < 0 during the pre-game lobby), advance to
    // Untap (0).
    const current = game.activePhase;
    const next = current >= 0 ? (current + 1) % 11 : 0;
    webClient.request.game.setActivePhase(gameId, { phase: next });
  };

  const handleConcedeToggle = () => {
    if (!hasLiveGame || (!canConcede && !canUnconcede)) {
      return;
    }
    if (isConceded) {
      onRequestUnconcede();
    } else {
      onRequestConcede();
    }
  };

  const handleRemoveArrows = () => {
    if (!canRemoveArrows) {
      return;
    }
    for (const arrowId of localArrowIds) {
      webClient.request.game.deleteArrow(gameId, { arrowId });
    }
  };

  const handleLeave = () => {
    if (!canLeave || !hasLiveGame) {
      return;
    }
    webClient.request.game.leaveGame(gameId);
  };

  const handleToggleInvert = () => {
    if (settingsStatus !== LoadingState.READY) {
      return;
    }
    void updateSettings({ invertVerticalCoordinate: !invertVerticalCoordinate });
  };

  const handleKick = (playerId: number) => {
    if (!hasLiveGame) {
      return;
    }
    webClient.request.game.kickFromGame(gameId, { playerId });
    setKickAnchor(null);
  };

  return (
    <div className="turn-controls" data-testid="turn-controls">
      <button
        type="button"
        className="turn-controls__btn"
        onClick={handlePassTurn}
        disabled={!canAdvance}
      >
        Pass Turn
      </button>
      <button
        type="button"
        className="turn-controls__btn"
        onClick={handleReverseTurn}
        disabled={!canAdvance}
      >
        Reverse Turn
      </button>
      <button
        type="button"
        className="turn-controls__btn"
        onClick={handleNextPhase}
        disabled={!canAdvance}
      >
        Next Phase
      </button>
      <button
        type="button"
        className="turn-controls__btn"
        onClick={handleConcedeToggle}
        disabled={!canConcede && !canUnconcede}
      >
        {isConceded ? 'Unconcede' : 'Concede'}
      </button>
      <button
        type="button"
        className="turn-controls__btn"
        onClick={onRequestRollDie}
        disabled={!canRoll}
      >
        Roll Die…
      </button>
      <button
        type="button"
        className="turn-controls__btn"
        onClick={handleRemoveArrows}
        disabled={!canRemoveArrows}
        title="Remove all arrows you've drawn this turn"
      >
        Remove Arrows
      </button>
      <button
        type="button"
        className={`turn-controls__btn${isRotated ? ' turn-controls__btn--active' : ''}`}
        onClick={onToggleRotate90}
        aria-pressed={isRotated}
        disabled={gameId == null}
        title="Rotate your view 90° (view-only; no server call)"
      >
        {isRotated ? 'Unrotate View' : 'Rotate 90°'}
      </button>
      <button
        type="button"
        className={`turn-controls__btn${invertVerticalCoordinate ? ' turn-controls__btn--active' : ''}`}
        onClick={handleToggleInvert}
        aria-pressed={invertVerticalCoordinate}
        disabled={settingsStatus !== LoadingState.READY}
        title="Flip battlefield row order (saved across sessions)"
      >
        Invert Rows
      </button>
      <button
        type="button"
        className="turn-controls__btn"
        onClick={onRequestGameInfo}
        disabled={!hasLiveGame}
      >
        Game Info
      </button>
      <button
        type="button"
        className="turn-controls__btn"
        onClick={handleLeave}
        disabled={!canLeave}
      >
        Leave Game
      </button>
      {isHost && (
        <>
          <button
            type="button"
            className="turn-controls__btn"
            onClick={(e) => setKickAnchor(e.currentTarget)}
            disabled={!canKick}
          >
            Kick ▾
          </button>
          <Menu
            open={kickAnchor != null}
            anchorEl={kickAnchor}
            onClose={() => setKickAnchor(null)}
          >
            {opponents.map((o) => (
              <MenuItem key={o.playerId} onClick={() => handleKick(o.playerId)}>
                {o.name}
              </MenuItem>
            ))}
          </Menu>
        </>
      )}
    </div>
  );
}

export default TurnControls;
