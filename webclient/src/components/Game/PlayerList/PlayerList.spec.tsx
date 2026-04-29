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
  it('lists every player in the game', () => {
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
    expect(screen.getByText('10s')).toBeInTheDocument();
    expect(screen.getByText('20s')).toBeInTheDocument();
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
