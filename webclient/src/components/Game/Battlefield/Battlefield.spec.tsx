import { screen } from '@testing-library/react';
import { App } from '@app/types';

vi.mock('../../../hooks/useSettings');

import { useSettings } from '../../../hooks/useSettings';
import { makeSettings, makeSettingsHook } from '../../../hooks/__mocks__/useSettings';
import { makeStoreState, renderWithProviders } from '../../../__test-utils__';
import {
  makeCard,
  makeGameEntry,
  makePlayerEntry,
  makeZoneEntry,
} from '../../../store/game/__mocks__/fixtures';
import Battlefield from './Battlefield';

function setInvert(invert: boolean) {
  vi.mocked(useSettings).mockReturnValue(
    makeSettingsHook({ value: makeSettings({ invertVerticalCoordinate: invert }) }),
  );
}

function stateWithBattlefield(cards: ReturnType<typeof makeCard>[]) {
  const table = makeZoneEntry({
    name: App.ZoneName.TABLE,
    type: 1,
    withCoords: true,
    cardCount: cards.length,
    cards,
  });
  const player = makePlayerEntry({
    zones: { [App.ZoneName.TABLE]: table },
  });
  const game = makeGameEntry({
    localPlayerId: 1,
    players: { 1: player },
  });
  return makeStoreState({ games: { games: { 1: game } } });
}

describe('Battlefield', () => {
  beforeEach(() => {
    vi.mocked(useSettings).mockReturnValue(makeSettingsHook());
  });

  it('renders three rows regardless of card count', () => {
    renderWithProviders(<Battlefield gameId={1} playerId={1} />, {
      preloadedState: stateWithBattlefield([]),
    });

    expect(screen.getByTestId('battlefield-row-0')).toBeInTheDocument();
    expect(screen.getByTestId('battlefield-row-1')).toBeInTheDocument();
    expect(screen.getByTestId('battlefield-row-2')).toBeInTheDocument();
  });

  it('places cards into rows by y coordinate', () => {
    const cards = [
      makeCard({ id: 1, name: 'Top', x: 0, y: 0 }),
      makeCard({ id: 2, name: 'Mid', x: 0, y: 1 }),
      makeCard({ id: 3, name: 'Bot', x: 0, y: 2 }),
    ];
    renderWithProviders(<Battlefield gameId={1} playerId={1} />, {
      preloadedState: stateWithBattlefield(cards),
    });

    expect(screen.getByTestId('battlefield-row-0').querySelector('img')?.alt).toBe('Top');
    expect(screen.getByTestId('battlefield-row-1').querySelector('img')?.alt).toBe('Mid');
    expect(screen.getByTestId('battlefield-row-2').querySelector('img')?.alt).toBe('Bot');
  });

  it('clamps out-of-range y values into the three-row space', () => {
    const cards = [
      makeCard({ id: 1, name: 'TooHigh', x: 0, y: -5 }),
      makeCard({ id: 2, name: 'TooLow', x: 0, y: 99 }),
    ];
    renderWithProviders(<Battlefield gameId={1} playerId={1} />, {
      preloadedState: stateWithBattlefield(cards),
    });

    expect(screen.getByTestId('battlefield-row-0').querySelector('img')?.alt).toBe('TooHigh');
    expect(screen.getByTestId('battlefield-row-2').querySelector('img')?.alt).toBe('TooLow');
  });

  it('sorts cards within a row by x coordinate', () => {
    const cards = [
      makeCard({ id: 1, name: 'Right', x: 10, y: 0 }),
      makeCard({ id: 2, name: 'Left', x: 0, y: 0 }),
    ];
    renderWithProviders(<Battlefield gameId={1} playerId={1} />, {
      preloadedState: stateWithBattlefield(cards),
    });

    const row0 = screen.getByTestId('battlefield-row-0');
    const imgs = Array.from(row0.querySelectorAll('img'));
    expect(imgs.map((i) => i.alt)).toEqual(['Left', 'Right']);
  });

  it('renders rows top-to-bottom as 0,1,2 when not mirrored', () => {
    const cards = [
      makeCard({ id: 1, name: 'A', x: 0, y: 0 }),
      makeCard({ id: 2, name: 'B', x: 0, y: 1 }),
      makeCard({ id: 3, name: 'C', x: 0, y: 2 }),
    ];
    renderWithProviders(<Battlefield gameId={1} playerId={1} />, {
      preloadedState: stateWithBattlefield(cards),
    });

    const rowsInOrder = Array.from(
      screen.getByTestId('battlefield').querySelectorAll('.battlefield__row'),
    );
    expect(rowsInOrder.map((r) => r.getAttribute('data-row'))).toEqual(['0', '1', '2']);
  });

  it('renders rows bottom-to-top as 2,1,0 when mirrored (opponent)', () => {
    const cards = [
      makeCard({ id: 1, name: 'A', x: 0, y: 0 }),
      makeCard({ id: 2, name: 'B', x: 0, y: 1 }),
      makeCard({ id: 3, name: 'C', x: 0, y: 2 }),
    ];
    renderWithProviders(<Battlefield gameId={1} playerId={1} mirrored />, {
      preloadedState: stateWithBattlefield(cards),
    });

    const rowsInOrder = Array.from(
      screen.getByTestId('battlefield').querySelectorAll('.battlefield__row'),
    );
    expect(rowsInOrder.map((r) => r.getAttribute('data-row'))).toEqual(['2', '1', '0']);
  });

  it('passes inverted=true to every CardSlot when mirrored', () => {
    const cards = [makeCard({ id: 1, name: 'A', x: 0, y: 0 })];
    const { container } = renderWithProviders(
      <Battlefield gameId={1} playerId={1} mirrored />,
      { preloadedState: stateWithBattlefield(cards) },
    );

    expect(container.querySelector('.card-slot--inverted')).not.toBeNull();
  });

  describe('invertVerticalCoordinate user setting', () => {
    it('renders rows bottom-to-top when the setting is on and not mirrored (local player)', () => {
      setInvert(true);
      const cards = [
        makeCard({ id: 1, name: 'A', x: 0, y: 0 }),
        makeCard({ id: 2, name: 'B', x: 0, y: 1 }),
        makeCard({ id: 3, name: 'C', x: 0, y: 2 }),
      ];
      renderWithProviders(<Battlefield gameId={1} playerId={1} />, {
        preloadedState: stateWithBattlefield(cards),
      });

      const rowsInOrder = Array.from(
        screen.getByTestId('battlefield').querySelectorAll('.battlefield__row'),
      );
      expect(rowsInOrder.map((r) => r.getAttribute('data-row'))).toEqual(['2', '1', '0']);
    });

    it('restores top-to-bottom ordering when setting is on AND mirrored (XOR cancels)', () => {
      setInvert(true);
      const cards = [
        makeCard({ id: 1, name: 'A', x: 0, y: 0 }),
        makeCard({ id: 2, name: 'B', x: 0, y: 1 }),
      ];
      renderWithProviders(<Battlefield gameId={1} playerId={1} mirrored />, {
        preloadedState: stateWithBattlefield(cards),
      });

      const rowsInOrder = Array.from(
        screen.getByTestId('battlefield').querySelectorAll('.battlefield__row'),
      );
      expect(rowsInOrder.map((r) => r.getAttribute('data-row'))).toEqual(['0', '1', '2']);
    });

    it('passes inverted=true to CardSlots when setting is on and not mirrored', () => {
      setInvert(true);
      const cards = [makeCard({ id: 1, name: 'A', x: 0, y: 0 })];
      const { container } = renderWithProviders(
        <Battlefield gameId={1} playerId={1} />,
        { preloadedState: stateWithBattlefield(cards) },
      );

      expect(container.querySelector('.card-slot--inverted')).not.toBeNull();
    });

    it('passes inverted=false to CardSlots when setting is on AND mirrored (XOR)', () => {
      setInvert(true);
      const cards = [makeCard({ id: 1, name: 'A', x: 0, y: 0 })];
      const { container } = renderWithProviders(
        <Battlefield gameId={1} playerId={1} mirrored />,
        { preloadedState: stateWithBattlefield(cards) },
      );

      expect(container.querySelector('.card-slot--inverted')).toBeNull();
    });
  });
});
