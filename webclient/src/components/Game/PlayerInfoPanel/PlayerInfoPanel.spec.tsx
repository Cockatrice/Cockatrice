import { screen, fireEvent } from '@testing-library/react';
import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';

import { createMockWebClient, makeStoreState, renderWithProviders, makeUser } from '../../../__test-utils__';
import {
  makeCounter,
  makeGameEntry,
  makePlayerEntry,
  makePlayerProperties,
} from '../../../store/game/__mocks__/fixtures';
import PlayerInfoPanel from './PlayerInfoPanel';

function statefulPlayer(
  overrides: Partial<Parameters<typeof makePlayerEntry>[0]> = {},
) {
  const player = makePlayerEntry({
    properties: makePlayerProperties({
      playerId: 1,
      userInfo: makeUser({ name: 'Pumuky' }),
      pingSeconds: 42,
    }),
    ...overrides,
  });
  return makeStoreState({
    games: {
      games: {
        1: makeGameEntry({
          localPlayerId: 1,
          players: { 1: player },
        }),
      },
    },
  });
}

describe('PlayerInfoPanel', () => {
  it('renders the player name and ping', () => {
    renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} />, {
      preloadedState: statefulPlayer(),
    });

    expect(screen.getByText('Pumuky')).toBeInTheDocument();
    expect(screen.getByText('42s')).toBeInTheDocument();
  });

  it('falls back to "(unknown)" when userInfo is absent', () => {
    const player = makePlayerEntry({
      properties: makePlayerProperties({ playerId: 1 }),
    });
    renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} />, {
      preloadedState: makeStoreState({
        games: { games: { 1: makeGameEntry({ players: { 1: player } }) } },
      }),
    });

    expect(screen.getByText('(unknown)')).toBeInTheDocument();
  });

  it('renders Life in a prominent block above the rest, with "LIFE" label', () => {
    const life = makeCounter({
      id: 1,
      name: 'Life',
      count: 20,
      counterColor: create(Data.colorSchema, { r: 255, g: 255, b: 255, a: 255 }),
    });
    const white = makeCounter({
      id: 2,
      name: 'W',
      count: 3,
      counterColor: create(Data.colorSchema, { r: 250, g: 245, b: 220, a: 255 }),
    });

    renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} />, {
      preloadedState: statefulPlayer({
        counters: { 1: life, 2: white },
      }),
    });

    expect(screen.getByTestId('life-1')).toHaveTextContent('20');
    expect(screen.getByText('LIFE')).toBeInTheDocument();
    // Other counters still render in the list with their name.
    expect(screen.getByText('W')).toBeInTheDocument();
    expect(screen.getByText('3')).toBeInTheDocument();
  });

  it('shows an empty-state line when no counters exist', () => {
    renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} />, {
      preloadedState: statefulPlayer({ counters: {} }),
    });

    expect(screen.getByText(/no counters/i)).toBeInTheDocument();
  });

  it('shows the Conceded flag when player has conceded', () => {
    const player = makePlayerEntry({
      properties: makePlayerProperties({
        playerId: 1,
        userInfo: makeUser({ name: 'Quitter' }),
        conceded: true,
      }),
    });
    renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} />, {
      preloadedState: makeStoreState({
        games: { games: { 1: makeGameEntry({ players: { 1: player } }) } },
      }),
    });

    expect(screen.getByText(/conceded/i)).toBeInTheDocument();
  });

  it('shows the Ready flag when readyStart is true and player has not conceded', () => {
    const player = makePlayerEntry({
      properties: makePlayerProperties({
        playerId: 1,
        userInfo: makeUser({ name: 'Waiting' }),
        readyStart: true,
      }),
    });
    renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} />, {
      preloadedState: makeStoreState({
        games: { games: { 1: makeGameEntry({ players: { 1: player } }) } },
      }),
    });

    expect(screen.getByText(/ready/i)).toBeInTheDocument();
  });

  it('renders an empty panel when the player is missing', () => {
    const { container } = renderWithProviders(
      <PlayerInfoPanel gameId={1} playerId={999} />,
      { preloadedState: statefulPlayer() },
    );

    expect(container.querySelector('.player-info-panel--empty')).not.toBeNull();
    expect(screen.queryByText('Pumuky')).not.toBeInTheDocument();
  });

  it('renders a host badge when the player is the game host', () => {
    const player = makePlayerEntry({
      properties: makePlayerProperties({
        playerId: 1,
        userInfo: makeUser({ name: 'Host' }),
      }),
    });
    const { container } = renderWithProviders(
      <PlayerInfoPanel gameId={1} playerId={1} />,
      {
        preloadedState: makeStoreState({
          games: {
            games: {
              1: makeGameEntry({ hostId: 1, players: { 1: player } }),
            },
          },
        }),
      },
    );

    expect(container.querySelector('.player-info-panel__host-badge')).not.toBeNull();
  });

  it('omits the host badge when the player is not the host', () => {
    const player = makePlayerEntry({
      properties: makePlayerProperties({
        playerId: 2,
        userInfo: makeUser({ name: 'Guest' }),
      }),
    });
    const { container } = renderWithProviders(
      <PlayerInfoPanel gameId={1} playerId={2} />,
      {
        preloadedState: makeStoreState({
          games: {
            games: {
              1: makeGameEntry({ hostId: 1, players: { 2: player } }),
            },
          },
        }),
      },
    );

    expect(container.querySelector('.player-info-panel__host-badge')).toBeNull();
  });

  // Sideboard lock indicator — mirrors desktop's `DeckViewContainer`
  // lock UI. The webclient surfaces it on the info panel since we don't
  // have a persistent deck view.
  it('renders a 🔒 indicator when player.properties.sideboardLocked is true', () => {
    const player = makePlayerEntry({
      properties: makePlayerProperties({
        playerId: 1,
        userInfo: makeUser({ name: 'P1' }),
        sideboardLocked: true,
      }),
    });
    renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} />, {
      preloadedState: makeStoreState({
        games: { games: { 1: makeGameEntry({ players: { 1: player } }) } },
      }),
    });

    expect(screen.getByLabelText('sideboard locked')).toBeInTheDocument();
  });

  it('omits the lock indicator when sideboardLocked is false', () => {
    const player = makePlayerEntry({
      properties: makePlayerProperties({
        playerId: 1,
        userInfo: makeUser({ name: 'P1' }),
        sideboardLocked: false,
      }),
    });
    renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} />, {
      preloadedState: makeStoreState({
        games: { games: { 1: makeGameEntry({ players: { 1: player } }) } },
      }),
    });

    expect(screen.queryByLabelText('sideboard locked')).not.toBeInTheDocument();
  });

  describe('editable counters', () => {
    const life = makeCounter({
      id: 1,
      name: 'Life',
      count: 20,
      counterColor: create(Data.colorSchema, { r: 255, g: 255, b: 255, a: 255 }),
    });

    it('does not render counter controls when canEdit is false (default)', () => {
      renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} />, {
        preloadedState: statefulPlayer({ counters: { 1: life } }),
      });

      expect(screen.queryByLabelText('increment Life')).not.toBeInTheDocument();
      expect(screen.queryByLabelText('decrement Life')).not.toBeInTheDocument();
      expect(screen.queryByLabelText('delete Life')).not.toBeInTheDocument();
    });

    it('renders +/− controls on the Life block when canEdit is true (Life has no delete — desktop parity)', () => {
      renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} canEdit />, {
        preloadedState: statefulPlayer({ counters: { 1: life } }),
      });

      expect(screen.getByLabelText('increment Life')).toBeInTheDocument();
      expect(screen.getByLabelText('decrement Life')).toBeInTheDocument();
      expect(screen.queryByLabelText('delete Life')).not.toBeInTheDocument();
    });

    it('dispatches incCounter(+1) when + is clicked', () => {
      const webClient = createMockWebClient();
      renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} canEdit />, {
        preloadedState: statefulPlayer({ counters: { 1: life } }),
        webClient,
      });

      fireEvent.click(screen.getByLabelText('increment Life'));

      expect(webClient.request.game.incCounter).toHaveBeenCalledWith(1, { counterId: 1, delta: 1 });
    });

    it('dispatches incCounter(-1) when − is clicked', () => {
      const webClient = createMockWebClient();
      renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} canEdit />, {
        preloadedState: statefulPlayer({ counters: { 1: life } }),
        webClient,
      });

      fireEvent.click(screen.getByLabelText('decrement Life'));

      expect(webClient.request.game.incCounter).toHaveBeenCalledWith(1, { counterId: 1, delta: -1 });
    });

    it('dispatches delCounter when × is clicked on a non-Life counter', () => {
      const webClient = createMockWebClient();
      const mana = makeCounter({
        id: 2,
        name: 'W',
        count: 3,
        counterColor: create(Data.colorSchema, { r: 255, g: 255, b: 255, a: 255 }),
      });
      renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} canEdit />, {
        preloadedState: statefulPlayer({ counters: { 2: mana } }),
        webClient,
      });

      fireEvent.click(screen.getByLabelText('delete W'));

      expect(webClient.request.game.delCounter).toHaveBeenCalledWith(1, { counterId: 2 });
    });

    it('swaps the value into an input on click and dispatches setCounter on Enter', () => {
      const webClient = createMockWebClient();
      renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} canEdit />, {
        preloadedState: statefulPlayer({ counters: { 1: life } }),
        webClient,
      });

      fireEvent.click(screen.getByText('20'));
      const input = screen.getByLabelText('set Life') as HTMLInputElement;
      fireEvent.change(input, { target: { value: '18' } });
      fireEvent.keyDown(input, { key: 'Enter' });

      expect(webClient.request.game.setCounter).toHaveBeenCalledWith(1, { counterId: 1, value: 18 });
    });

    it('does not dispatch setCounter when Escape is pressed during inline edit', () => {
      const webClient = createMockWebClient();
      renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} canEdit />, {
        preloadedState: statefulPlayer({ counters: { 1: life } }),
        webClient,
      });

      fireEvent.click(screen.getByText('20'));
      const input = screen.getByLabelText('set Life') as HTMLInputElement;
      fireEvent.change(input, { target: { value: '99' } });
      fireEvent.keyDown(input, { key: 'Escape' });

      expect(webClient.request.game.setCounter).not.toHaveBeenCalled();
    });

    it('fires onRequestCreateCounter when "+ New counter" is clicked', () => {
      const onRequestCreateCounter = vi.fn();
      renderWithProviders(
        <PlayerInfoPanel
          gameId={1}
          playerId={1}
          canEdit
          onRequestCreateCounter={onRequestCreateCounter}
        />,
        { preloadedState: statefulPlayer({ counters: { 1: life } }) },
      );

      fireEvent.click(screen.getByText('+ New counter'));

      expect(onRequestCreateCounter).toHaveBeenCalled();
    });

    it('does not render the new-counter button when canEdit is false', () => {
      renderWithProviders(
        <PlayerInfoPanel gameId={1} playerId={1} onRequestCreateCounter={() => {}} />,
        { preloadedState: statefulPlayer({ counters: {} }) },
      );

      expect(screen.queryByText('+ New counter')).not.toBeInTheDocument();
    });
  });
});
