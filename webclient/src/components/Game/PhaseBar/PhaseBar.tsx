import Tooltip from '@mui/material/Tooltip';

import { App } from '@app/types';
import { cx } from '@app/utils';

import { usePhaseBar } from './usePhaseBar';

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
  const { activePhase, canAdvance, handlePhaseClick, handlePass, handleUntapAll, handleDrawOne } =
    usePhaseBar(gameId);

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
