import { screen, fireEvent } from '@testing-library/react';
import { App } from '@app/types';

import { makeStoreState, renderWithProviders } from '../../../__test-utils__';
import {
  makeGameEntry,
  makePlayerEntry,
  makeZoneEntry,
} from '../../../store/game/__mocks__/fixtures';
import ZoneRail from './ZoneRail';

const baseState = makeStoreState({
  games: {
    games: {
      1: makeGameEntry({
        localPlayerId: 1,
        players: {
          1: makePlayerEntry({
            zones: {
              [App.ZoneName.STACK]: makeZoneEntry({ name: App.ZoneName.STACK }),
              [App.ZoneName.EXILE]: makeZoneEntry({ name: App.ZoneName.EXILE }),
              [App.ZoneName.GRAVE]: makeZoneEntry({ name: App.ZoneName.GRAVE }),
              [App.ZoneName.DECK]: makeZoneEntry({ name: App.ZoneName.DECK, cardCount: 60 }),
            },
          }),
        },
      }),
    },
  },
});

describe('ZoneRail', () => {
  it('renders deck, graveyard, and exile top-to-bottom (desktop pile order)', () => {
    const { container } = renderWithProviders(<ZoneRail gameId={1} playerId={1} />, {
      preloadedState: baseState,
    });

    const labels = Array.from(container.querySelectorAll('.zone-stack__label')).map(
      (n) => n.textContent,
    );
    expect(labels).toEqual(['Deck', 'Graveyard', 'Exile']);
  });

  it('does not render the stack in the pile rail (desktop parity: stack is not a pile)', () => {
    renderWithProviders(<ZoneRail gameId={1} playerId={1} />, {
      preloadedState: baseState,
    });

    expect(screen.queryByText('Stack')).not.toBeInTheDocument();
    expect(screen.queryByTestId(`zone-stack-${App.ZoneName.STACK}`)).not.toBeInTheDocument();
  });

  it('propagates player and game context to each ZoneStack', () => {
    renderWithProviders(<ZoneRail gameId={1} playerId={1} />, {
      preloadedState: baseState,
    });

    expect(screen.getByTestId(`zone-stack-${App.ZoneName.DECK}`)).toBeInTheDocument();
    expect(screen.getByTestId(`zone-stack-${App.ZoneName.GRAVE}`)).toBeInTheDocument();
    expect(screen.getByTestId(`zone-stack-${App.ZoneName.EXILE}`)).toBeInTheDocument();
  });

  it('forwards zone-rail clicks with the player and zone name when onZoneClick is provided', () => {
    const onZoneClick = vi.fn();
    renderWithProviders(
      <ZoneRail gameId={1} playerId={7} onZoneClick={onZoneClick} />,
      { preloadedState: baseState },
    );

    fireEvent.click(screen.getByTestId(`zone-stack-${App.ZoneName.GRAVE}`));

    expect(onZoneClick).toHaveBeenCalledWith(7, App.ZoneName.GRAVE);
  });
});
