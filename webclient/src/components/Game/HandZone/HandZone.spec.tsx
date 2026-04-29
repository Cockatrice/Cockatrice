import { screen, fireEvent } from '@testing-library/react';
import { App } from '@app/types';

import { makeStoreState, renderWithProviders } from '../../../__test-utils__';
import {
  makeCard,
  makeGameEntry,
  makePlayerEntry,
  makeZoneEntry,
} from '../../../store/game/__mocks__/fixtures';
import HandZone from './HandZone';

function stateWithHand(cards: ReturnType<typeof makeCard>[]) {
  const hand = makeZoneEntry({
    name: App.ZoneName.HAND,
    type: 0,
    cardCount: cards.length,
    cards,
  });
  return makeStoreState({
    games: {
      games: {
        1: makeGameEntry({
          localPlayerId: 1,
          players: {
            1: makePlayerEntry({ zones: { [App.ZoneName.HAND]: hand } }),
          },
        }),
      },
    },
  });
}

describe('HandZone', () => {
  it('renders the hand label with the current count', () => {
    const cards = [
      makeCard({ id: 1, name: 'Island' }),
      makeCard({ id: 2, name: 'Swamp' }),
    ];
    renderWithProviders(<HandZone gameId={1} playerId={1} />, {
      preloadedState: stateWithHand(cards),
    });

    expect(screen.getByText(/Hand · 2/)).toBeInTheDocument();
  });

  it('renders a CardSlot for every card in hand', () => {
    const cards = [
      makeCard({ id: 1, name: 'Forest' }),
      makeCard({ id: 2, name: 'Mountain' }),
    ];
    renderWithProviders(<HandZone gameId={1} playerId={1} />, {
      preloadedState: stateWithHand(cards),
    });

    expect(screen.getAllByTestId('card-slot')).toHaveLength(2);
    expect(screen.getByAltText('Forest')).toBeInTheDocument();
    expect(screen.getByAltText('Mountain')).toBeInTheDocument();
  });

  it('renders an empty row when hand is empty', () => {
    renderWithProviders(<HandZone gameId={1} playerId={1} />, {
      preloadedState: stateWithHand([]),
    });

    expect(screen.getByText(/Hand · 0/)).toBeInTheDocument();
    expect(screen.queryAllByTestId('card-slot')).toHaveLength(0);
  });

  describe('zone-level context menu', () => {
    it('fires onZoneContextMenu when right-clicking the empty hand area', () => {
      const onZoneContextMenu = vi.fn();
      renderWithProviders(
        <HandZone gameId={1} playerId={1} onZoneContextMenu={onZoneContextMenu} />,
        { preloadedState: stateWithHand([]) },
      );

      fireEvent.contextMenu(screen.getByTestId('hand-zone'));

      expect(onZoneContextMenu).toHaveBeenCalled();
    });

    it('does NOT fire onZoneContextMenu when right-clicking a card slot', () => {
      const onZoneContextMenu = vi.fn();
      const cards = [makeCard({ id: 1, name: 'Island' })];
      renderWithProviders(
        <HandZone gameId={1} playerId={1} onZoneContextMenu={onZoneContextMenu} />,
        { preloadedState: stateWithHand(cards) },
      );

      fireEvent.contextMenu(screen.getByTestId('card-slot'));

      expect(onZoneContextMenu).not.toHaveBeenCalled();
    });
  });
});
