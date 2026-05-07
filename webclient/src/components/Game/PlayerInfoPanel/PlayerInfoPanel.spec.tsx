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
  it('renders the player name', () => {
    renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} />, {
      preloadedState: statefulPlayer(),
    });

    expect(screen.getByText('Pumuky')).toBeInTheDocument();
  });

  it('does not render ping (ping lives in the right-side PlayerList)', () => {
    renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} />, {
      preloadedState: statefulPlayer(),
    });

    expect(screen.queryByText('42s')).not.toBeInTheDocument();
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

  it('renders the Life counter inline in the header and other counters as circles in the body', () => {
    const life = makeCounter({
      id: 1,
      name: 'Life',
      count: 20,
      counterColor: create(Data.colorSchema, { r: 255, g: 150, b: 0, a: 255 }),
    });
    const white = makeCounter({
      id: 2,
      name: 'W',
      count: 3,
      counterColor: create(Data.colorSchema, { r: 250, g: 245, b: 220, a: 255 }),
    });

    const { container } = renderWithProviders(
      <PlayerInfoPanel gameId={1} playerId={1} />,
      {
        preloadedState: statefulPlayer({ counters: { 2: white, 1: life } }),
      },
    );

    const lifePill = screen.getByTestId('counter-1');
    const whitePill = screen.getByTestId('counter-2');

    expect(lifePill).toHaveTextContent('20');
    expect(whitePill).toHaveTextContent('3');
    expect(lifePill).toHaveClass('player-info-panel__counter--life');
    expect(whitePill).not.toHaveClass('player-info-panel__counter--life');

    // Life sits inside the header (life-slot); others sit in the body list.
    expect(container.querySelector('.player-info-panel__header')?.contains(lifePill)).toBe(true);
    expect(container.querySelector('.player-info-panel__counters')?.contains(whitePill)).toBe(true);
    expect(container.querySelector('.player-info-panel__counters')?.contains(lifePill)).toBe(false);
  });

  it('renders no counter elements when the player has no counters', () => {
    renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} />, {
      preloadedState: statefulPlayer({ counters: {} }),
    });

    expect(screen.queryByTestId(/^counter-/)).not.toBeInTheDocument();
  });

  it('renders the Deck, Hand, Graveyard, and Exile zones inside the body', () => {
    renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} />, {
      preloadedState: statefulPlayer(),
    });

    expect(screen.getByTestId('zone-stack-deck')).toBeInTheDocument();
    expect(screen.getByTestId('zone-stack-hand')).toBeInTheDocument();
    expect(screen.getByTestId('zone-stack-grave')).toBeInTheDocument();
    expect(screen.getByTestId('zone-stack-rfg')).toBeInTheDocument();
  });

  it('does not wire left-click onZoneClick for the Hand row', () => {
    const onZoneClick = vi.fn();
    renderWithProviders(
      <PlayerInfoPanel gameId={1} playerId={1} onZoneClick={onZoneClick} />,
      { preloadedState: statefulPlayer() },
    );

    fireEvent.click(screen.getByTestId('zone-stack-hand'));

    expect(onZoneClick).not.toHaveBeenCalled();
  });

  it('forwards zone clicks with (playerId, zoneName)', () => {
    const onZoneClick = vi.fn();
    renderWithProviders(
      <PlayerInfoPanel gameId={1} playerId={1} onZoneClick={onZoneClick} />,
      { preloadedState: statefulPlayer() },
    );

    fireEvent.click(screen.getByTestId('zone-stack-deck'));

    expect(onZoneClick).toHaveBeenCalledWith(1, 'deck');
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

  // Sideboard lock lives in the right-side PlayerList — it must NOT render
  // inside the PlayerInfoPanel.
  it('never renders the sideboard-lock indicator in the panel', () => {
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

    expect(screen.queryByLabelText('sideboard locked')).not.toBeInTheDocument();
  });

  describe('counter color fallback', () => {
    it('falls back to the MTG name map when the server omits the color', () => {
      const white = makeCounter({
        id: 1,
        name: 'W',
        count: 1,
        counterColor: create(Data.colorSchema, { r: 0, g: 0, b: 0, a: 0 }),
      });
      renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} />, {
        preloadedState: statefulPlayer({ counters: { 1: white } }),
      });

      expect(screen.getByTestId('counter-1')).toHaveStyle({
        background: 'rgba(245, 245, 220, 1)',
      });
    });

    it('respects the server color when it is present and non-zero', () => {
      const custom = makeCounter({
        id: 2,
        name: 'W',
        count: 1,
        counterColor: create(Data.colorSchema, { r: 10, g: 20, b: 30, a: 255 }),
      });
      renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} />, {
        preloadedState: statefulPlayer({ counters: { 2: custom } }),
      });

      expect(screen.getByTestId('counter-2')).toHaveStyle({
        background: 'rgba(10, 20, 30, 1)',
      });
    });

    it('derives a stable non-black hash color for unknown names', () => {
      const poison1 = makeCounter({
        id: 3,
        name: 'Poison',
        count: 1,
        counterColor: create(Data.colorSchema, { r: 0, g: 0, b: 0, a: 0 }),
      });
      const { unmount } = renderWithProviders(
        <PlayerInfoPanel gameId={1} playerId={1} />,
        { preloadedState: statefulPlayer({ counters: { 3: poison1 } }) },
      );
      const first = screen.getByTestId('counter-3').getAttribute('style') ?? '';
      unmount();

      renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} />, {
        preloadedState: statefulPlayer({ counters: { 3: poison1 } }),
      });
      const second = screen.getByTestId('counter-3').getAttribute('style') ?? '';

      expect(first).toBe(second);
      // jsdom normalizes rgba(r,g,b,1) → rgb(r,g,b) on style attributes; accept either.
      expect(first).toMatch(/rgba?\(\d+, \d+, \d+(?:, 1)?\)/);
      expect(first).not.toMatch(/rgba?\(0, 0, 0/);
    });
  });

  describe('editable counters', () => {
    const life = makeCounter({
      id: 1,
      name: 'Life',
      count: 20,
      counterColor: create(Data.colorSchema, { r: 255, g: 255, b: 255, a: 255 }),
    });

    it('does not attach click handlers when canEdit is false (default)', () => {
      const webClient = createMockWebClient();
      renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} />, {
        preloadedState: statefulPlayer({ counters: { 1: life } }),
        webClient,
      });

      const pill = screen.getByTestId('counter-1');
      expect(pill).not.toHaveAttribute('role', 'button');

      fireEvent.click(pill);
      fireEvent.contextMenu(pill);
      expect(webClient.request.game.incCounter).not.toHaveBeenCalled();
    });

    it('dispatches incCounter(+1) on left-click when canEdit is true', () => {
      const webClient = createMockWebClient();
      renderWithProviders(<PlayerInfoPanel gameId={1} playerId={1} canEdit />, {
        preloadedState: statefulPlayer({ counters: { 1: life } }),
        webClient,
      });

      fireEvent.click(screen.getByTestId('counter-1'));

      expect(webClient.request.game.incCounter).toHaveBeenCalledWith(1, { counterId: 1, delta: 1 });
    });

    it('dispatches incCounter(-1) on right-click, suppresses browser menu, and does not bubble to the panel', () => {
      const webClient = createMockWebClient();
      const onContextMenu = vi.fn();
      renderWithProviders(
        <PlayerInfoPanel gameId={1} playerId={1} canEdit onContextMenu={onContextMenu} />,
        {
          preloadedState: statefulPlayer({ counters: { 1: life } }),
          webClient,
        },
      );

      const dispatched = fireEvent.contextMenu(screen.getByTestId('counter-1'));

      expect(dispatched).toBe(false);
      expect(webClient.request.game.incCounter).toHaveBeenCalledWith(1, { counterId: 1, delta: -1 });
      expect(onContextMenu).not.toHaveBeenCalled();
    });

    it('still fires the panel-level onContextMenu when right-clicking outside a counter', () => {
      const onContextMenu = vi.fn();
      const { container } = renderWithProviders(
        <PlayerInfoPanel gameId={1} playerId={1} canEdit onContextMenu={onContextMenu} />,
        { preloadedState: statefulPlayer({ counters: { 1: life } }) },
      );

      fireEvent.contextMenu(container.querySelector('.player-info-panel__name')!);

      expect(onContextMenu).toHaveBeenCalled();
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
