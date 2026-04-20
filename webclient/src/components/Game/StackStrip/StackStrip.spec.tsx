import { screen, fireEvent } from '@testing-library/react';
import { App } from '@app/types';

import { makeStoreState, renderWithProviders } from '../../../__test-utils__';
import {
  makeGameEntry,
  makePlayerEntry,
  makePlayerProperties,
  makeZoneEntry,
} from '../../../store/game/__mocks__/fixtures';
import StackStrip from './StackStrip';

function stateWithStacks(localCount: number, opponentCount: number) {
  const local = makePlayerEntry({
    properties: makePlayerProperties({ playerId: 1 }),
    zones: {
      [App.ZoneName.STACK]: makeZoneEntry({
        name: App.ZoneName.STACK,
        cardCount: localCount,
      }),
    },
  });
  const opponent = makePlayerEntry({
    properties: makePlayerProperties({ playerId: 2 }),
    zones: {
      [App.ZoneName.STACK]: makeZoneEntry({
        name: App.ZoneName.STACK,
        cardCount: opponentCount,
      }),
    },
  });
  return makeStoreState({
    games: {
      games: {
        1: makeGameEntry({ localPlayerId: 1, players: { 1: local, 2: opponent } }),
      },
    },
  });
}

describe('StackStrip', () => {
  it('renders a cell per entry with the zone cardCount', () => {
    renderWithProviders(
      <StackStrip
        gameId={1}
        entries={[
          { playerId: 2, name: 'Opp' },
          { playerId: 1, name: 'Me' },
        ]}
      />,
      { preloadedState: stateWithStacks(0, 3) },
    );

    const oppCell = screen.getByTestId('stack-strip-cell-2');
    const meCell = screen.getByTestId('stack-strip-cell-1');
    expect(oppCell).toHaveTextContent('Opp');
    expect(oppCell).toHaveTextContent('3');
    expect(meCell).toHaveTextContent('Me');
    expect(meCell).toHaveTextContent('0');
  });

  it('invokes onZoneClick(playerId, "stack") when a cell is clicked', () => {
    const onZoneClick = vi.fn();
    renderWithProviders(
      <StackStrip
        gameId={1}
        entries={[
          { playerId: 2, name: 'Opp' },
          { playerId: 1, name: 'Me' },
        ]}
        onZoneClick={onZoneClick}
      />,
      { preloadedState: stateWithStacks(1, 2) },
    );

    fireEvent.click(screen.getByTestId('stack-strip-cell-1'));

    expect(onZoneClick).toHaveBeenCalledWith(1, App.ZoneName.STACK);
  });

  it('activates on Enter/Space when clickable', () => {
    const onZoneClick = vi.fn();
    renderWithProviders(
      <StackStrip
        gameId={1}
        entries={[{ playerId: 1, name: 'Me' }]}
        onZoneClick={onZoneClick}
      />,
      { preloadedState: stateWithStacks(0, 0) },
    );

    fireEvent.keyDown(screen.getByTestId('stack-strip-cell-1'), { key: 'Enter' });

    expect(onZoneClick).toHaveBeenCalledWith(1, App.ZoneName.STACK);
  });

  it('renders cells as non-interactive when onZoneClick is absent', () => {
    renderWithProviders(
      <StackStrip gameId={1} entries={[{ playerId: 1, name: 'Me' }]} />,
      { preloadedState: stateWithStacks(0, 0) },
    );

    const cell = screen.getByTestId('stack-strip-cell-1');
    expect(cell).not.toHaveAttribute('role', 'button');
    expect(cell).not.toHaveAttribute('tabindex');
  });
});
