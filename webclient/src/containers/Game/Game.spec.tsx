import { screen, fireEvent, waitFor } from '@testing-library/react';
import { App } from '@app/types';

import { Data } from '@app/types';

import { createMockWebClient, makeStoreState, renderWithProviders, connectedState, makeUser } from '../../__test-utils__';
import {
  makeCard,
  makeGameEntry,
  makePlayerEntry,
  makePlayerProperties,
  makeZoneEntry,
} from '../../store/game/__mocks__/fixtures';
import Game from './Game';

// Layout pulls in LeftNav which is not under test here; stub to a no-op.
vi.mock('../Layout/Layout', () => ({
  default: ({ children }: { children: React.ReactNode }) => <>{children}</>,
}));

// Block TurnControls' / Battlefield's Dexie-backed useSettings from firing
// an async settle after mount (would produce an unwrapped React state update).
vi.mock('../../hooks/useSettings');

interface BuildGameOpts {
  localId: number;
  opponentIds: number[];
  tableCards?: ReturnType<typeof makeCard>[];
  started?: boolean;
  spectator?: boolean;
  judge?: boolean;
  localReadyStart?: boolean;
  graveCards?: ReturnType<typeof makeCard>[];
}

function buildGame({
  localId,
  opponentIds,
  tableCards = [],
  started = true,
  spectator = false,
  judge = false,
  localReadyStart = false,
  graveCards = [],
}: BuildGameOpts) {
  const players: Record<number, ReturnType<typeof makePlayerEntry>> = {};
  const playerIds = [localId, ...opponentIds];
  for (const pid of playerIds) {
    players[pid] = makePlayerEntry({
      properties: makePlayerProperties({
        playerId: pid,
        userInfo: makeUser({ name: `P${pid}` }),
        readyStart: pid === localId ? localReadyStart : false,
      }),
      zones: {
        [App.ZoneName.TABLE]: makeZoneEntry({
          name: App.ZoneName.TABLE,
          cards: pid === localId ? tableCards : [],
          cardCount: pid === localId ? tableCards.length : 0,
        }),
        [App.ZoneName.HAND]: makeZoneEntry({ name: App.ZoneName.HAND }),
        [App.ZoneName.DECK]: makeZoneEntry({ name: App.ZoneName.DECK, cardCount: 40 }),
        [App.ZoneName.GRAVE]: makeZoneEntry({
          name: App.ZoneName.GRAVE,
          cards: pid === localId ? graveCards : [],
          cardCount: pid === localId ? graveCards.length : 0,
        }),
        [App.ZoneName.EXILE]: makeZoneEntry({ name: App.ZoneName.EXILE }),
      },
    });
  }
  return makeStoreState({
    ...connectedState,
    games: {
      games: {
        1: makeGameEntry({
          localPlayerId: localId,
          spectator,
          judge,
          started,
          players,
        }),
      },
    },
  });
}

describe('Game container', () => {
  it('shows the empty-game placeholder when no game is active', () => {
    renderWithProviders(<Game />, {
      preloadedState: makeStoreState({
        ...connectedState,
        games: { games: {} },
      }),
    });

    expect(screen.getByTestId('game-empty')).toBeInTheDocument();
    expect(screen.getByTestId('phase-bar')).toBeInTheDocument();
    expect(screen.getByTestId('right-panel')).toBeInTheDocument();
  });

  it('renders both player boards and the hand when a 2-player game is active', () => {
    renderWithProviders(<Game />, {
      preloadedState: buildGame({ localId: 1, opponentIds: [2] }),
    });

    expect(screen.getByTestId('player-board-1')).toBeInTheDocument();
    expect(screen.getByTestId('player-board-2')).toBeInTheDocument();
    expect(screen.getByTestId('hand-zone')).toBeInTheDocument();
    expect(screen.queryByTestId('game-empty')).not.toBeInTheDocument();
  });

  it('does not render the opponent selector in a 2-player game', () => {
    renderWithProviders(<Game />, {
      preloadedState: buildGame({ localId: 1, opponentIds: [2] }),
    });

    expect(screen.queryByTestId('opponent-selector')).not.toBeInTheDocument();
  });

  it('renders the opponent selector in a 3+ player game', () => {
    renderWithProviders(<Game />, {
      preloadedState: buildGame({ localId: 1, opponentIds: [2, 3] }),
    });

    expect(screen.getByTestId('opponent-selector')).toBeInTheDocument();
  });

  it('defaults to showing the first opponent', () => {
    renderWithProviders(<Game />, {
      preloadedState: buildGame({ localId: 1, opponentIds: [2, 3] }),
    });

    expect(screen.getByTestId('player-board-2')).toBeInTheDocument();
    expect(screen.queryByTestId('player-board-3')).not.toBeInTheDocument();
  });

  it('mirrors the opponent board and leaves the local board upright', () => {
    renderWithProviders(<Game />, {
      preloadedState: buildGame({ localId: 1, opponentIds: [2] }),
    });

    expect(screen.getByTestId('player-board-2')).toHaveClass('player-board--mirrored');
    expect(screen.getByTestId('player-board-1')).not.toHaveClass('player-board--mirrored');
  });

  it('lifts card-hover state into the right panel preview', () => {
    const card = makeCard({ id: 7, name: 'Lightning Bolt', x: 0, y: 0 });
    renderWithProviders(<Game />, {
      preloadedState: buildGame({
        localId: 1,
        opponentIds: [2],
        tableCards: [card],
      }),
    });

    const small = document.querySelector('.card-preview__image--small') as HTMLImageElement | null;
    expect(small).toBeNull();

    const slot = screen.getAllByTestId('card-slot')[0];
    fireEvent.mouseEnter(slot);

    const afterHover = document.querySelector('.card-preview__image--small') as HTMLImageElement;
    expect(afterHover.src).toContain('Lightning%20Bolt');
  });

  it('keeps the phase bar and right panel visible when no game is joined', () => {
    renderWithProviders(<Game />, {
      preloadedState: makeStoreState({
        ...connectedState,
        games: { games: {} },
      }),
    });

    expect(screen.getByTestId('phase-bar')).toBeInTheDocument();
    expect(screen.getByTestId('right-panel')).toBeInTheDocument();
  });

  describe('DeckSelectDialog auto-open', () => {
    it('opens automatically when game is not started and local player is not ready', () => {
      renderWithProviders(<Game />, {
        preloadedState: buildGame({
          localId: 1,
          opponentIds: [2],
          started: false,
          localReadyStart: false,
        }),
      });

      expect(screen.getByRole('dialog')).toBeInTheDocument();
      expect(screen.getByLabelText('deck list')).toBeInTheDocument();
    });

    it('stays closed when the game has already started', () => {
      renderWithProviders(<Game />, {
        preloadedState: buildGame({
          localId: 1,
          opponentIds: [2],
          started: true,
          localReadyStart: false,
        }),
      });

      expect(screen.queryByLabelText('deck list')).not.toBeInTheDocument();
    });

    it('stays closed once the local player is ready', () => {
      renderWithProviders(<Game />, {
        preloadedState: buildGame({
          localId: 1,
          opponentIds: [2],
          started: false,
          localReadyStart: true,
        }),
      });

      expect(screen.queryByLabelText('deck list')).not.toBeInTheDocument();
    });

    it('stays closed for spectators', () => {
      renderWithProviders(<Game />, {
        preloadedState: buildGame({
          localId: 1,
          opponentIds: [2],
          started: false,
          spectator: true,
        }),
      });

      expect(screen.queryByLabelText('deck list')).not.toBeInTheDocument();
    });

    it('stays closed for judges', () => {
      renderWithProviders(<Game />, {
        preloadedState: buildGame({
          localId: 1,
          opponentIds: [2],
          started: false,
          judge: true,
        }),
      });

      expect(screen.queryByLabelText('deck list')).not.toBeInTheDocument();
    });

    // Judges on Servatrice are flagged spectator on the wire. Both gates
    // independently suppress the deck-select dialog; this pins that either
    // one alone is sufficient.
    it('stays closed for judges who are also flagged as spectators', () => {
      renderWithProviders(<Game />, {
        preloadedState: buildGame({
          localId: 1,
          opponentIds: [2],
          started: false,
          judge: true,
          spectator: true,
        }),
      });

      expect(screen.queryByLabelText('deck list')).not.toBeInTheDocument();
    });
  });

  describe('ZoneViewDialog', () => {
    it('is closed by default', () => {
      renderWithProviders(<Game />, {
        preloadedState: buildGame({ localId: 1, opponentIds: [2] }),
      });

      expect(screen.queryByRole('button', { name: /close zone view/i })).not.toBeInTheDocument();
    });

    it('opens when a zone-rail entry is clicked, showing the cards in that zone', () => {
      const graveCard = makeCard({ id: 77, name: 'Final Card' });
      renderWithProviders(<Game />, {
        preloadedState: buildGame({
          localId: 1,
          opponentIds: [2],
          graveCards: [graveCard],
        }),
      });

      const localBoard = screen.getByTestId('player-board-1');
      const graveStack = localBoard.querySelector(`[data-testid="zone-stack-${App.ZoneName.GRAVE}"]`)!;
      fireEvent.click(graveStack);

      expect(screen.getByRole('button', { name: /close zone view/i })).toBeInTheDocument();
      expect(screen.getAllByAltText('Final Card').length).toBeGreaterThan(0);
    });

    it('closes when the close button is clicked', async () => {
      renderWithProviders(<Game />, {
        preloadedState: buildGame({ localId: 1, opponentIds: [2] }),
      });

      const graveStack = screen
        .getByTestId('player-board-1')
        .querySelector(`[data-testid="zone-stack-${App.ZoneName.GRAVE}"]`)!;
      fireEvent.click(graveStack);
      fireEvent.click(screen.getByRole('button', { name: /close zone view/i }));

      await waitFor(() => {
        expect(screen.queryByRole('button', { name: /close zone view/i })).not.toBeInTheDocument();
      });
    });

    // Click propagation from the opponent's grave uses the same handler as
    // the local grave; M2 deferrable wanted this pinned explicitly so a
    // regression that scopes the click to the local board only is caught.
    it('opens the opponent-owned grave and titles the panel with the opponent name', () => {
      renderWithProviders(<Game />, {
        preloadedState: buildGame({ localId: 1, opponentIds: [2] }),
      });

      const opponentBoard = screen.getByTestId('player-board-2');
      const graveStack = opponentBoard.querySelector(`[data-testid="zone-stack-${App.ZoneName.GRAVE}"]`)!;
      fireEvent.click(graveStack);

      const panel = screen.getByTestId('zone-view-dialog');
      expect(panel).toHaveAttribute('aria-label', expect.stringMatching(/P2 Graveyard/));
    });
  });

  describe('Card interactions (M3)', () => {
    it('double-clicking a battlefield card toggles tap via setCardAttr', () => {
      const webClient = createMockWebClient();
      const card = makeCard({ id: 7, name: 'Creature', x: 0, y: 0, tapped: false });
      renderWithProviders(<Game />, {
        preloadedState: buildGame({
          localId: 1,
          opponentIds: [2],
          tableCards: [card],
        }),
        webClient,
      });

      const localBoard = screen.getByTestId('player-board-1');
      const slot = localBoard.querySelector('[data-testid="card-slot"]')!;
      fireEvent.doubleClick(slot);

      expect(webClient.request.game.setCardAttr).toHaveBeenCalledWith(1, {
        zone: App.ZoneName.TABLE,
        cardId: 7,
        attribute: Data.CardAttribute.AttrTapped,
        attrValue: '1',
      });
    });

    it('right-clicking a local card opens the card context menu', () => {
      const card = makeCard({ id: 7, name: 'Creature', x: 0, y: 0 });
      renderWithProviders(<Game />, {
        preloadedState: buildGame({
          localId: 1,
          opponentIds: [2],
          tableCards: [card],
        }),
      });

      const slot = screen.getByTestId('player-board-1').querySelector('[data-testid="card-slot"]')!;
      fireEvent.contextMenu(slot);

      expect(screen.getByText('Flip')).toBeInTheDocument();
      expect(screen.getByText('Tap')).toBeInTheDocument();
    });

    it('right-clicking the local deck opens the zone context menu', () => {
      renderWithProviders(<Game />, {
        preloadedState: buildGame({ localId: 1, opponentIds: [2] }),
      });

      const localDeck = screen
        .getByTestId('player-board-1')
        .querySelector(`[data-testid="zone-stack-${App.ZoneName.DECK}"]`)!;
      fireEvent.contextMenu(localDeck);

      expect(screen.getByText('Draw a card')).toBeInTheDocument();
      expect(screen.getByText('Shuffle')).toBeInTheDocument();
    });

    it('does NOT open a zone context menu for the opponent deck', () => {
      renderWithProviders(<Game />, {
        preloadedState: buildGame({ localId: 1, opponentIds: [2] }),
      });

      const opponentDeck = screen
        .getByTestId('player-board-2')
        .querySelector(`[data-testid="zone-stack-${App.ZoneName.DECK}"]`)!;
      fireEvent.contextMenu(opponentDeck);

      expect(screen.queryByText('Draw a card')).not.toBeInTheDocument();
    });

    it('opening a card menu closes an already-open zone menu', () => {
      const card = makeCard({ id: 7, x: 0, y: 0 });
      renderWithProviders(<Game />, {
        preloadedState: buildGame({
          localId: 1,
          opponentIds: [2],
          tableCards: [card],
        }),
      });

      const localDeck = screen
        .getByTestId('player-board-1')
        .querySelector(`[data-testid="zone-stack-${App.ZoneName.DECK}"]`)!;
      fireEvent.contextMenu(localDeck);
      expect(screen.getByText('Draw a card')).toBeInTheDocument();

      const slot = screen.getByTestId('player-board-1').querySelector('[data-testid="card-slot"]')!;
      fireEvent.contextMenu(slot);

      expect(screen.queryByText('Draw a card')).not.toBeInTheDocument();
      expect(screen.getByText('Flip')).toBeInTheDocument();
    });

    it('dispatches drawCards(1) when "Draw a card" is chosen from the deck menu', () => {
      const webClient = createMockWebClient();
      renderWithProviders(<Game />, {
        preloadedState: buildGame({ localId: 1, opponentIds: [2] }),
        webClient,
      });

      fireEvent.contextMenu(
        screen.getByTestId('player-board-1').querySelector(`[data-testid="zone-stack-${App.ZoneName.DECK}"]`)!,
      );
      fireEvent.click(screen.getByText('Draw a card'));

      expect(webClient.request.game.drawCards).toHaveBeenCalledWith(1, { number: 1 });
    });

    it('opens a PromptDialog when "Set P/T…" is chosen and dispatches setCardAttr on submit', () => {
      const webClient = createMockWebClient();
      const card = makeCard({ id: 7, x: 0, y: 0, pt: '' });
      renderWithProviders(<Game />, {
        preloadedState: buildGame({
          localId: 1,
          opponentIds: [2],
          tableCards: [card],
        }),
        webClient,
      });

      fireEvent.contextMenu(
        screen.getByTestId('player-board-1').querySelector('[data-testid="card-slot"]')!,
      );
      fireEvent.click(screen.getByText('Set P/T…'));

      const input = screen.getByLabelText('P/T (e.g. 3/3)');
      fireEvent.change(input, { target: { value: '3/3' } });
      fireEvent.click(screen.getByRole('button', { name: /ok/i }));

      expect(webClient.request.game.setCardAttr).toHaveBeenCalledWith(1, {
        zone: App.ZoneName.TABLE,
        cardId: 7,
        attribute: Data.CardAttribute.AttrPT,
        attrValue: '3/3',
      });
    });
  });

  // M4–M6 orchestration tests live in Game.orchestration.spec.tsx — that
  // file pins the end-to-end dispatch flows (dialog/menu → command) that go
  // through Game.tsx state wiring. Splitting them out lets vitest's threads
  // pool run them in parallel with the unit-style tests in this file.

});
