import { screen } from '@testing-library/react';
import { App } from '@app/types';

// Block Battlefield's Dexie-backed useSettings from firing an async settle
// after mount (would produce an unwrapped React state update).
vi.mock('../../../hooks/useSettings');

import { makeStoreState, renderWithProviders, makeUser } from '../../../__test-utils__';
import {
  makeCard,
  makeGameEntry,
  makePlayerEntry,
  makePlayerProperties,
  makeZoneEntry,
} from '../../../store/game/__mocks__/fixtures';
import PlayerBoard from './PlayerBoard';

function buildState() {
  const table = makeZoneEntry({
    name: App.ZoneName.TABLE,
    cards: [
      makeCard({ id: 1, name: 'Row0-Card', x: 0, y: 0 }),
      makeCard({ id: 2, name: 'Row2-Card', x: 0, y: 2 }),
    ],
    cardCount: 2,
  });
  const hand = makeZoneEntry({ name: App.ZoneName.HAND, cardCount: 0 });
  const deck = makeZoneEntry({ name: App.ZoneName.DECK, cardCount: 60 });
  const player = makePlayerEntry({
    properties: makePlayerProperties({
      playerId: 1,
      userInfo: makeUser({ name: 'Trajer' }),
    }),
    zones: {
      [App.ZoneName.TABLE]: table,
      [App.ZoneName.HAND]: hand,
      [App.ZoneName.DECK]: deck,
    },
  });
  return makeStoreState({
    games: {
      games: {
        1: makeGameEntry({ localPlayerId: 1, players: { 1: player } }),
      },
    },
  });
}

describe('PlayerBoard', () => {
  it('renders the info panel, battlefield, and zone rail in order', () => {
    renderWithProviders(<PlayerBoard gameId={1} playerId={1} />, {
      preloadedState: buildState(),
    });

    expect(screen.getByText('Trajer')).toBeInTheDocument();
    expect(screen.getByTestId('battlefield')).toBeInTheDocument();
    expect(screen.getByTestId('zone-rail')).toBeInTheDocument();
  });

  it('passes mirrored=false by default so the battlefield uses natural row order', () => {
    const { container } = renderWithProviders(
      <PlayerBoard gameId={1} playerId={1} />,
      { preloadedState: buildState() },
    );

    const rowsInOrder = Array.from(
      container.querySelectorAll('.battlefield__row'),
    ).map((r) => r.getAttribute('data-row'));
    expect(rowsInOrder).toEqual(['0', '1', '2']);
    expect(container.querySelector('.card-slot--inverted')).toBeNull();
  });

  it('propagates mirrored=true → battlefield reverses row order and cards are inverted', () => {
    const { container } = renderWithProviders(
      <PlayerBoard gameId={1} playerId={1} mirrored />,
      { preloadedState: buildState() },
    );

    const rowsInOrder = Array.from(
      container.querySelectorAll('.battlefield__row'),
    ).map((r) => r.getAttribute('data-row'));
    expect(rowsInOrder).toEqual(['2', '1', '0']);
    expect(container.querySelectorAll('.card-slot--inverted').length).toBeGreaterThan(0);
  });

  it('keeps the info panel on the left and zone rail on the right in mirrored mode', () => {
    const { container } = renderWithProviders(
      <PlayerBoard gameId={1} playerId={1} mirrored />,
      { preloadedState: buildState() },
    );

    const children = Array.from(container.querySelector('.player-board')!.children);
    expect(children[0]).toHaveClass('player-info-panel');
    expect(children[1]).toHaveClass('battlefield');
    expect(children[2]).toHaveClass('zone-rail');
  });

  it('adds the --mirrored CSS modifier only when mirrored', () => {
    const { container, rerender } = renderWithProviders(
      <PlayerBoard gameId={1} playerId={1} />,
      { preloadedState: buildState() },
    );

    expect(container.querySelector('.player-board--mirrored')).toBeNull();
    rerender(<PlayerBoard gameId={1} playerId={1} mirrored />);
    expect(container.querySelector('.player-board--mirrored')).not.toBeNull();
  });
});
