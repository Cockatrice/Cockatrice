import { screen } from '@testing-library/react';

import { makeStoreState, renderWithProviders, makeUser } from '../../../__test-utils__';
import {
  makeGameEntry,
  makePlayerEntry,
  makePlayerProperties,
} from '../../../store/game/__mocks__/fixtures';
import PlayerList from './PlayerList';

function buildState(
  players: ReturnType<typeof makePlayerEntry>[],
  activePlayerId: number,
  hostId?: number,
) {
  const byId: Record<number, ReturnType<typeof makePlayerEntry>> = {};
  for (const p of players) {
    byId[p.properties.playerId] = p;
  }
  return makeStoreState({
    games: {
      games: {
        1: makeGameEntry({
          players: byId,
          activePlayerId,
          ...(hostId != null ? { hostId } : {}),
        }),
      },
    },
  });
}

describe('PlayerList', () => {
  it('lists every player in the game with a ping-dot tooltip', () => {
    const p1 = makePlayerEntry({
      properties: makePlayerProperties({
        playerId: 1,
        userInfo: makeUser({ name: 'Alice' }),
        pingSeconds: 10,
      }),
    });
    const p2 = makePlayerEntry({
      properties: makePlayerProperties({
        playerId: 2,
        userInfo: makeUser({ name: 'Bob' }),
        pingSeconds: 20,
      }),
    });

    renderWithProviders(<PlayerList gameId={1} />, {
      preloadedState: buildState([p1, p2], 1),
    });

    expect(screen.getByText('Alice')).toBeInTheDocument();
    expect(screen.getByText('Bob')).toBeInTheDocument();
    expect(screen.getByLabelText('ping 10s')).toBeInTheDocument();
    expect(screen.getByLabelText('ping 20s')).toBeInTheDocument();
    // Raw-seconds text no longer renders; the dot carries the info via tooltip.
    expect(screen.queryByText('10s')).not.toBeInTheDocument();
    expect(screen.queryByText('20s')).not.toBeInTheDocument();
  });

  describe('ping-dot color', () => {
    it('colors a low ping green', () => {
      const p = makePlayerEntry({
        properties: makePlayerProperties({
          playerId: 1,
          userInfo: makeUser({ name: 'Alice' }),
          pingSeconds: 0,
        }),
      });
      renderWithProviders(<PlayerList gameId={1} />, {
        preloadedState: buildState([p], 1),
      });

      expect(screen.getByTestId('ping-dot-1')).toHaveStyle({
        background: 'hsl(120, 100%, 50%)',
      });
    });

    it('colors a saturated ping red (clamped at 10s)', () => {
      const p = makePlayerEntry({
        properties: makePlayerProperties({
          playerId: 1,
          userInfo: makeUser({ name: 'Alice' }),
          pingSeconds: 15,
        }),
      });
      renderWithProviders(<PlayerList gameId={1} />, {
        preloadedState: buildState([p], 1),
      });

      expect(screen.getByTestId('ping-dot-1')).toHaveStyle({
        background: 'hsl(0, 100%, 50%)',
      });
    });

    it('colors a disconnected player black (ping < 0)', () => {
      const p = makePlayerEntry({
        properties: makePlayerProperties({
          playerId: 1,
          userInfo: makeUser({ name: 'Alice' }),
          pingSeconds: -1,
        }),
      });
      renderWithProviders(<PlayerList gameId={1} />, {
        preloadedState: buildState([p], 1),
      });

      expect(screen.getByTestId('ping-dot-1')).toHaveStyle({
        background: '#000',
      });
    });
  });

  describe('sideboard lock', () => {
    it('shows the 🔒 icon for a player with a locked sideboard', () => {
      const p = makePlayerEntry({
        properties: makePlayerProperties({
          playerId: 1,
          userInfo: makeUser({ name: 'Alice' }),
          sideboardLocked: true,
        }),
      });
      renderWithProviders(<PlayerList gameId={1} />, {
        preloadedState: buildState([p], 1),
      });

      expect(screen.getByLabelText('sideboard locked')).toBeInTheDocument();
    });

    it('hides the 🔒 icon for a player with an unlocked sideboard', () => {
      const p = makePlayerEntry({
        properties: makePlayerProperties({
          playerId: 1,
          userInfo: makeUser({ name: 'Alice' }),
          sideboardLocked: false,
        }),
      });
      renderWithProviders(<PlayerList gameId={1} />, {
        preloadedState: buildState([p], 1),
      });

      expect(screen.queryByLabelText('sideboard locked')).not.toBeInTheDocument();
    });
  });

  it('highlights the active player', () => {
    const p1 = makePlayerEntry({
      properties: makePlayerProperties({
        playerId: 1,
        userInfo: makeUser({ name: 'Alice' }),
      }),
    });
    const p2 = makePlayerEntry({
      properties: makePlayerProperties({
        playerId: 2,
        userInfo: makeUser({ name: 'Bob' }),
      }),
    });

    renderWithProviders(<PlayerList gameId={1} />, {
      preloadedState: buildState([p1, p2], 2),
    });

    expect(screen.getByTestId('player-list-item-2')).toHaveClass(
      'player-list__item--active',
    );
    expect(screen.getByTestId('player-list-item-1')).not.toHaveClass(
      'player-list__item--active',
    );
  });

  it('dims conceded players', () => {
    const p1 = makePlayerEntry({
      properties: makePlayerProperties({
        playerId: 1,
        userInfo: makeUser({ name: 'Alice' }),
        conceded: true,
      }),
    });

    renderWithProviders(<PlayerList gameId={1} />, {
      preloadedState: buildState([p1], 0),
    });

    expect(screen.getByTestId('player-list-item-1')).toHaveClass(
      'player-list__item--conceded',
    );
  });

  it('shows empty state when there are no players', () => {
    renderWithProviders(<PlayerList gameId={1} />, {
      preloadedState: buildState([], 0),
    });

    expect(screen.getByText(/no players/i)).toBeInTheDocument();
  });

  it('handles missing gameId without throwing', () => {
    renderWithProviders(<PlayerList gameId={undefined} />, {
      preloadedState: makeStoreState({}),
    });

    expect(screen.getByText(/no players/i)).toBeInTheDocument();
  });

  it('renders a host badge on the host row only', () => {
    const p1 = makePlayerEntry({
      properties: makePlayerProperties({
        playerId: 1,
        userInfo: makeUser({ name: 'Alice' }),
      }),
    });
    const p2 = makePlayerEntry({
      properties: makePlayerProperties({
        playerId: 2,
        userInfo: makeUser({ name: 'Bob' }),
      }),
    });

    renderWithProviders(<PlayerList gameId={1} />, {
      preloadedState: buildState([p1, p2], 1, 2),
    });

    const bobRow = screen.getByTestId('player-list-item-2');
    const aliceRow = screen.getByTestId('player-list-item-1');
    expect(bobRow.querySelector('.player-list__host-badge')).not.toBeNull();
    expect(aliceRow.querySelector('.player-list__host-badge')).toBeNull();
  });
});
