import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';

import { useTurnControls } from './useTurnControls';

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
  const {
    isHost,
    isConceded,
    invertVerticalCoordinate,
    settingsReady,
    canAdvance,
    canLeave,
    canConcede,
    canUnconcede,
    canRoll,
    canKick,
    canRemoveArrows,
    hasLiveGame,
    opponents,
    kickAnchor,
    setKickAnchor,
    handlePassTurn,
    handleReverseTurn,
    handleNextPhase,
    handleConcedeToggle,
    handleRemoveArrows,
    handleLeave,
    handleToggleInvert,
    handleKick,
  } = useTurnControls({ gameId, onRequestConcede, onRequestUnconcede });

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
        disabled={!settingsReady}
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
