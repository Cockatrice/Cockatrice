import { screen, fireEvent } from '@testing-library/react';
import { App } from '@app/types';

import { makeStoreState, renderWithProviders, makeUser } from '../../__test-utils__';
import {
  makeCard,
  makeGameEntry,
  makePlayerEntry,
  makePlayerProperties,
  makeZoneEntry,
} from '../../store/game/__mocks__/fixtures';
import ZoneViewDialog from './ZoneViewDialog';

function stateWith(zone: Parameters<typeof makeZoneEntry>[0]) {
  return makeStoreState({
    games: {
      games: {
        1: makeGameEntry({
          localPlayerId: 1,
          players: {
            1: makePlayerEntry({
              properties: makePlayerProperties({
                playerId: 1,
                userInfo: makeUser({ name: 'Trajer' }),
              }),
              zones: {
                [zone.name!]: makeZoneEntry(zone),
              },
            }),
          },
        }),
      },
    },
  });
}

describe('ZoneViewDialog', () => {
  it('does not render content when closed', () => {
    renderWithProviders(
      <ZoneViewDialog
        isOpen={false}
        gameId={1}
        playerId={1}
        zoneName={App.ZoneName.GRAVE}
        handleClose={() => {}}
      />,
      { preloadedState: stateWith({ name: App.ZoneName.GRAVE, cardCount: 0 }) },
    );

    expect(screen.queryByTestId('zone-view-dialog')).not.toBeInTheDocument();
  });

  it('renders each card in the zone with its Scryfall image', () => {
    const cards = [
      makeCard({ id: 1, name: 'Lightning Bolt' }),
      makeCard({ id: 2, name: 'Counterspell' }),
    ];
    renderWithProviders(
      <ZoneViewDialog
        isOpen
        gameId={1}
        playerId={1}
        zoneName={App.ZoneName.GRAVE}
        handleClose={() => {}}
      />,
      {
        preloadedState: stateWith({
          name: App.ZoneName.GRAVE,
          cards,
          cardCount: 2,
        }),
      },
    );

    expect(screen.getByAltText('Lightning Bolt')).toBeInTheDocument();
    expect(screen.getByAltText('Counterspell')).toBeInTheDocument();
  });

  it('shows the zone label in the title with the player name and count', () => {
    renderWithProviders(
      <ZoneViewDialog
        isOpen
        gameId={1}
        playerId={1}
        zoneName={App.ZoneName.GRAVE}
        handleClose={() => {}}
      />,
      {
        preloadedState: stateWith({
          name: App.ZoneName.GRAVE,
          cards: [makeCard({ id: 1 })],
          cardCount: 1,
        }),
      },
    );

    expect(screen.getByText(/Trajer Graveyard \(1\)/)).toBeInTheDocument();
  });

  it('shows "This zone is empty." when the zone is fully empty', () => {
    renderWithProviders(
      <ZoneViewDialog
        isOpen
        gameId={1}
        playerId={1}
        zoneName={App.ZoneName.EXILE}
        handleClose={() => {}}
      />,
      { preloadedState: stateWith({ name: App.ZoneName.EXILE, cardCount: 0 }) },
    );

    expect(screen.getByText(/this zone is empty/i)).toBeInTheDocument();
  });

  it('shows a hidden-card fallback for hidden zones with count > 0 and no visible cards', () => {
    renderWithProviders(
      <ZoneViewDialog
        isOpen
        gameId={1}
        playerId={1}
        zoneName={App.ZoneName.DECK}
        handleClose={() => {}}
      />,
      {
        preloadedState: stateWith({
          name: App.ZoneName.DECK,
          cardCount: 40,
          cards: [],
        }),
      },
    );

    expect(screen.getByText(/40 hidden cards/i)).toBeInTheDocument();
  });

  it('hides the image and shows "Face Down" for face-down cards', () => {
    const faceDown = makeCard({ id: 1, name: 'Secret', faceDown: true });
    renderWithProviders(
      <ZoneViewDialog
        isOpen
        gameId={1}
        playerId={1}
        zoneName={App.ZoneName.EXILE}
        handleClose={() => {}}
      />,
      {
        preloadedState: stateWith({
          name: App.ZoneName.EXILE,
          cards: [faceDown],
          cardCount: 1,
        }),
      },
    );

    expect(screen.getByText('Face Down')).toBeInTheDocument();
    expect(screen.queryByAltText('Secret')).not.toBeInTheDocument();
  });

  it('fires handleClose when the ✕ button is clicked', () => {
    const handleClose = vi.fn();
    renderWithProviders(
      <ZoneViewDialog
        isOpen
        gameId={1}
        playerId={1}
        zoneName={App.ZoneName.GRAVE}
        handleClose={handleClose}
      />,
      { preloadedState: stateWith({ name: App.ZoneName.GRAVE, cardCount: 0 }) },
    );

    fireEvent.click(screen.getByRole('button', { name: /close zone view/i }));

    expect(handleClose).toHaveBeenCalled();
  });

  it('renders a non-modal floating panel at the provided initial position', () => {
    renderWithProviders(
      <ZoneViewDialog
        isOpen
        gameId={1}
        playerId={1}
        zoneName={App.ZoneName.GRAVE}
        handleClose={() => {}}
        initialPosition={{ x: 200, y: 150 }}
      />,
      { preloadedState: stateWith({ name: App.ZoneName.GRAVE, cardCount: 0 }) },
    );

    const panel = screen.getByTestId('zone-view-dialog');
    expect(panel).toHaveStyle({ left: '200px', top: '150px' });
    expect(panel).toHaveAttribute('role', 'dialog');
  });
});
