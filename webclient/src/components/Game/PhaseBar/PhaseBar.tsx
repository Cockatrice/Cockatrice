import Tooltip from '@mui/material/Tooltip';

import { useCurrentGame, useWebClient } from '@app/hooks';
import { GameSelectors, useAppSelector } from '@app/store';
import { App, Data } from '@app/types';
import { cx } from '@app/utils';

import './PhaseBar.css';

export interface PhaseBarProps {
  gameId: number | undefined;
}

// Abbreviated phase badges plus full tooltip titles. Desktop uses full
// words in the horizontal toolbar; we shorten for the vertical strip but
// keep the full text available on hover per the tooltip deferrable.
const PHASE_LABELS: ReadonlyArray<{
  phase: App.Phase;
  label: string;
  title: string;
  /** Desktop phase buttons wire "Untap All" to phase 0 double-click and "Draw a Card" to phase 2 double-click. */
  builtInOnDoubleClick?: 'untapAll' | 'drawCard';
}> = [
  { phase: App.Phase.Untap, label: 'UNTAP', title: 'Untap step (double-click: untap all)', builtInOnDoubleClick: 'untapAll' },
  { phase: App.Phase.Upkeep, label: 'UPKP', title: 'Upkeep step' },
  { phase: App.Phase.Draw, label: 'DRAW', title: 'Draw step (double-click: draw a card)', builtInOnDoubleClick: 'drawCard' },
  { phase: App.Phase.FirstMain, label: 'M1', title: 'First main phase' },
  { phase: App.Phase.BeginCombat, label: 'CMBT', title: 'Beginning of combat' },
  { phase: App.Phase.DeclareAttackers, label: 'ATTK', title: 'Declare attackers' },
  { phase: App.Phase.DeclareBlockers, label: 'BLCK', title: 'Declare blockers' },
  { phase: App.Phase.CombatDamage, label: 'DMGE', title: 'Combat damage' },
  { phase: App.Phase.EndCombat, label: 'ECMB', title: 'End of combat' },
  { phase: App.Phase.SecondMain, label: 'M2', title: 'Second main phase' },
  { phase: App.Phase.EndCleanup, label: 'END', title: 'End step / cleanup' },
];

function PhaseBar({ gameId }: PhaseBarProps) {
  const webClient = useWebClient();
  const { game, isJudge, isStarted } = useCurrentGame(gameId);
  const activePhase = useAppSelector((state) =>
    gameId != null ? GameSelectors.getActivePhase(state, gameId) : undefined,
  );
  const localPlayerId = game?.localPlayerId;
  const tableCards = useAppSelector((state) =>
    gameId != null && localPlayerId != null
      ? GameSelectors.getCards(state, gameId, localPlayerId, App.ZoneName.TABLE)
      : undefined,
  );

  // Desktop: only the active player (or a judge) can advance the phase.
  const canAdvance =
    gameId != null &&
    game != null &&
    isStarted &&
    (isJudge || game.activePlayerId === game.localPlayerId);

  const handlePhaseClick = (phase: App.Phase) => {
    if (!canAdvance || gameId == null) {
      return;
    }
    webClient.request.game.setActivePhase(gameId, { phase });
  };

  const handlePass = () => {
    if (!canAdvance || gameId == null) {
      return;
    }
    webClient.request.game.nextTurn(gameId);
  };

  // Desktop's untap-step double-click fires "Untap All" on the local player's
  // table zone (cockatrice/src/game/player/player_actions.cpp actUntapAll).
  // We replicate by sending one setCardAttr per tapped card; there is no
  // batch variant on the wire.
  const handleUntapAll = () => {
    if (!canAdvance || gameId == null || !tableCards) {
      return;
    }
    for (const card of tableCards) {
      if (card.tapped) {
        webClient.request.game.setCardAttr(gameId, {
          zone: App.ZoneName.TABLE,
          cardId: card.id,
          attribute: Data.CardAttribute.AttrTapped,
          attrValue: '0',
        });
      }
    }
  };

  const handleDrawOne = () => {
    if (!canAdvance || gameId == null) {
      return;
    }
    webClient.request.game.drawCards(gameId, { number: 1 });
  };

  const onDoubleClickFor = (kind: 'untapAll' | 'drawCard' | undefined) => {
    if (kind === 'untapAll') {
      return handleUntapAll;
    }
    if (kind === 'drawCard') {
      return handleDrawOne;
    }
    return undefined;
  };

  return (
    <nav className="phase-bar" data-testid="phase-bar" aria-label="Turn phases">
      {PHASE_LABELS.map(({ phase, label, title, builtInOnDoubleClick }) => {
        const isActive = phase === activePhase;
        return (
          <Tooltip key={phase} title={title} placement="right" enterDelay={500}>
            {/* span wrapper: MUI Tooltip can't attach listeners to a disabled button. */}
            <span className="phase-bar__btn-wrap">
              <button
                type="button"
                className={cx('phase-bar__btn', { 'phase-bar__btn--active': isActive })}
                data-phase={phase}
                disabled={!canAdvance}
                onClick={() => handlePhaseClick(phase)}
                onDoubleClick={onDoubleClickFor(builtInOnDoubleClick)}
              >
                {label}
              </button>
            </span>
          </Tooltip>
        );
      })}
      <div className="phase-bar__spacer" />
      <Tooltip title="Pass to the next turn" placement="right" enterDelay={500}>
        <span className="phase-bar__btn-wrap">
          <button
            type="button"
            className="phase-bar__btn phase-bar__btn--pass"
            disabled={!canAdvance}
            onClick={handlePass}
          >
            PASS TURN
          </button>
        </span>
      </Tooltip>
    </nav>
  );
}

export default PhaseBar;
