import { screen, fireEvent } from '@testing-library/react';
import { App } from '@app/types';

import { makeStoreState, renderWithProviders } from '../../../__test-utils__';
import {
  makeCard,
  makeGameEntry,
  makePlayerEntry,
  makeZoneEntry,
} from '../../../store/game/__mocks__/fixtures';
import ZoneStack from './ZoneStack';

function stateWithZone(zoneName: string, overrides: Parameters<typeof makeZoneEntry>[0]) {
  return makeStoreState({
    games: {
      games: {
        1: makeGameEntry({
          localPlayerId: 1,
          players: {
            1: makePlayerEntry({
              zones: {
                [zoneName]: makeZoneEntry({ name: zoneName, ...overrides }),
              },
            }),
          },
        }),
      },
    },
  });
}

describe('ZoneStack', () => {
  it('renders the label', () => {
    renderWithProviders(
      <ZoneStack gameId={1} playerId={1} zoneName={App.ZoneName.GRAVE} label="Graveyard" />,
      { preloadedState: stateWithZone(App.ZoneName.GRAVE, { cardCount: 0 }) },
    );

    expect(screen.getByText('Graveyard')).toBeInTheDocument();
  });

  it('shows the authoritative cardCount, even when order is empty (hidden zone)', () => {
    renderWithProviders(
      <ZoneStack gameId={1} playerId={1} zoneName={App.ZoneName.DECK} label="Deck" />,
      {
        preloadedState: stateWithZone(App.ZoneName.DECK, {
          cardCount: 40,
          cards: [],
        }),
      },
    );

    expect(screen.getByText('40')).toBeInTheDocument();
  });

  it('renders the top (last) card image as the thumb', () => {
    const a = makeCard({ id: 1, name: 'Bottom Card' });
    const b = makeCard({ id: 2, name: 'Top Card' });
    renderWithProviders(
      <ZoneStack gameId={1} playerId={1} zoneName={App.ZoneName.GRAVE} label="Graveyard" />,
      {
        preloadedState: stateWithZone(App.ZoneName.GRAVE, {
          cardCount: 2,
          cards: [a, b],
        }),
      },
    );

    expect(screen.getByAltText('Top Card')).toBeInTheDocument();
    expect(screen.queryByAltText('Bottom Card')).not.toBeInTheDocument();
  });

  it('renders a placeholder when the zone has no visible cards', () => {
    const { container } = renderWithProviders(
      <ZoneStack gameId={1} playerId={1} zoneName={App.ZoneName.EXILE} label="Exile" />,
      { preloadedState: stateWithZone(App.ZoneName.EXILE, { cardCount: 0 }) },
    );

    expect(container.querySelector('.zone-stack__placeholder')).not.toBeNull();
    expect(container.querySelector('.zone-stack__image')).toBeNull();
  });

  it('hides the image when the top card is face-down', () => {
    const hidden = makeCard({ id: 1, name: 'Secret', faceDown: true });
    const { container } = renderWithProviders(
      <ZoneStack gameId={1} playerId={1} zoneName={App.ZoneName.EXILE} label="Exile" />,
      {
        preloadedState: stateWithZone(App.ZoneName.EXILE, {
          cardCount: 1,
          cards: [hidden],
        }),
      },
    );

    expect(container.querySelector('.zone-stack__placeholder')).not.toBeNull();
    expect(container.querySelector('.zone-stack__image')).toBeNull();
  });

  it('renders count 0 when the zone entry is missing entirely', () => {
    renderWithProviders(
      <ZoneStack gameId={1} playerId={1} zoneName="nonexistent" label="Missing" />,
      {
        preloadedState: makeStoreState({
          games: {
            games: {
              1: makeGameEntry({
                players: { 1: makePlayerEntry({ zones: {} }) },
              }),
            },
          },
        }),
      },
    );

    expect(screen.getByText('0')).toBeInTheDocument();
  });

  it('fires onClick with the zone name when clicked', () => {
    const onClick = vi.fn();
    renderWithProviders(
      <ZoneStack
        gameId={1}
        playerId={1}
        zoneName={App.ZoneName.GRAVE}
        label="Graveyard"
        onClick={onClick}
      />,
      { preloadedState: stateWithZone(App.ZoneName.GRAVE, { cardCount: 0 }) },
    );

    fireEvent.click(screen.getByTestId(`zone-stack-${App.ZoneName.GRAVE}`));

    expect(onClick).toHaveBeenCalledWith(App.ZoneName.GRAVE);
  });

  it('does not gain button semantics when onClick is omitted', () => {
    renderWithProviders(
      <ZoneStack gameId={1} playerId={1} zoneName={App.ZoneName.GRAVE} label="Graveyard" />,
      { preloadedState: stateWithZone(App.ZoneName.GRAVE, { cardCount: 0 }) },
    );

    const el = screen.getByTestId(`zone-stack-${App.ZoneName.GRAVE}`);
    expect(el).not.toHaveAttribute('role', 'button');
    expect(el).not.toHaveAttribute('tabindex');
  });

  it.each([['Enter'], [' ']])('fires onClick on %s keypress when focusable', (key) => {
    const onClick = vi.fn();
    renderWithProviders(
      <ZoneStack
        gameId={1}
        playerId={1}
        zoneName={App.ZoneName.GRAVE}
        label="Graveyard"
        onClick={onClick}
      />,
      { preloadedState: stateWithZone(App.ZoneName.GRAVE, { cardCount: 0 }) },
    );

    fireEvent.keyDown(screen.getByTestId(`zone-stack-${App.ZoneName.GRAVE}`), { key });
    expect(onClick).toHaveBeenCalledWith(App.ZoneName.GRAVE);
  });
});
