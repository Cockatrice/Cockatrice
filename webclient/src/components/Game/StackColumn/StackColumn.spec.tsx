import { screen } from '@testing-library/react';
import { App } from '@app/types';

vi.mock('../../../hooks/useSettings');
vi.mock('../../../hooks/useScryfallCard', () => ({
  useScryfallCard: () => ({ smallUrl: null, normalUrl: null, isLoading: false }),
}));

import { makeStoreState, renderWithProviders, makeUser } from '../../../__test-utils__';
import {
  makeCard,
  makeGameEntry,
  makePlayerEntry,
  makePlayerProperties,
  makeZoneEntry,
} from '../../../store/game/__mocks__/fixtures';
import StackColumn from './StackColumn';

function stateWithStack(cards: ReturnType<typeof makeCard>[] = []) {
  const stack = makeZoneEntry({
    name: App.ZoneName.STACK,
    cards,
    cardCount: cards.length,
  });
  const player = makePlayerEntry({
    properties: makePlayerProperties({
      playerId: 1,
      userInfo: makeUser({ name: 'Alice' }),
    }),
    zones: { [App.ZoneName.STACK]: stack },
  });
  return makeStoreState({
    games: { games: { 1: makeGameEntry({ localPlayerId: 1, players: { 1: player } }) } },
  });
}

describe('StackColumn', () => {
  it('renders an empty cards container when the stack is empty', () => {
    renderWithProviders(<StackColumn gameId={1} playerId={1} />, {
      preloadedState: stateWithStack([]),
    });

    expect(screen.getByTestId('stack-column-1')).toBeInTheDocument();
    const cards = screen.getByTestId('stack-column-cards-1');
    expect(cards.children).toHaveLength(0);
  });

  it('renders one thumbnail per card on the stack', () => {
    renderWithProviders(<StackColumn gameId={1} playerId={1} />, {
      preloadedState: stateWithStack([
        makeCard({ id: 1, name: 'Lightning Bolt' }),
        makeCard({ id: 2, name: 'Counterspell' }),
      ]),
    });

    const cards = screen.getByTestId('stack-column-cards-1');
    expect(cards.children).toHaveLength(2);
  });
});
