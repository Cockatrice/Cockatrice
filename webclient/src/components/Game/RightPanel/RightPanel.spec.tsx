import { screen, fireEvent } from '@testing-library/react';

// Block TurnControls' Dexie-backed useSettings from firing an async settle
// after mount (the Dexie mock resolves on a microtask, which would produce
// an unwrapped React state update inside TurnControls).
vi.mock('../../../hooks/useSettings');

import { makeStoreState, renderWithProviders } from '../../../__test-utils__';
import { makeCard, makeGameEntry } from '../../../store/game/__mocks__/fixtures';
import RightPanel from './RightPanel';

function stateWithGame() {
  return makeStoreState({ games: { games: { 1: makeGameEntry() } } });
}

const NOOP = () => {};
const DEFAULT_RP_PROPS = {
  gameId: 1,
  hoveredCard: null,
  onRequestRollDie: NOOP,
  onRequestConcede: NOOP,
  onRequestUnconcede: NOOP,
  onRequestGameInfo: NOOP,
  onToggleRotate90: NOOP,
  isRotated: false,
};

describe('RightPanel', () => {
  it('renders CardPreview, PlayerList, GameLog, and TurnControls', () => {
    renderWithProviders(<RightPanel {...DEFAULT_RP_PROPS} />, {
      preloadedState: stateWithGame(),
    });

    expect(screen.getByTestId('card-preview')).toBeInTheDocument();
    expect(screen.getByTestId('player-list')).toBeInTheDocument();
    expect(screen.getByTestId('game-log')).toBeInTheDocument();
    expect(screen.getByTestId('turn-controls')).toBeInTheDocument();
  });

  it('forwards the hovered card into the preview', () => {
    const card = makeCard({ name: 'Lightning Bolt' });
    renderWithProviders(
      <RightPanel {...DEFAULT_RP_PROPS} hoveredCard={card} />,
      { preloadedState: stateWithGame() },
    );

    const small = document.querySelector('.card-preview__image--small') as HTMLImageElement;
    expect(small.src).toContain('Lightning%20Bolt');
  });

  it('forwards Roll Die clicks through to the parent callback', () => {
    const onRequestRollDie = vi.fn();
    renderWithProviders(
      <RightPanel {...DEFAULT_RP_PROPS} onRequestRollDie={onRequestRollDie} />,
      { preloadedState: stateWithGame() },
    );

    fireEvent.click(screen.getByRole('button', { name: /roll die/i }));

    expect(onRequestRollDie).toHaveBeenCalled();
  });

  it('shows the Spectating tag when the local user is a spectator', () => {
    renderWithProviders(<RightPanel {...DEFAULT_RP_PROPS} />, {
      preloadedState: makeStoreState({
        games: { games: { 1: makeGameEntry({ spectator: true }) } },
      }),
    });

    expect(screen.getByTestId('spectating-tag')).toBeInTheDocument();
  });

  it('hides the Spectating tag when the local user is a participant', () => {
    renderWithProviders(<RightPanel {...DEFAULT_RP_PROPS} />, {
      preloadedState: stateWithGame(),
    });

    expect(screen.queryByTestId('spectating-tag')).not.toBeInTheDocument();
  });
});
