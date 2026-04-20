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

  // M4–M6 orchestration — each of these goes through Game.tsx state wiring
  // between a trigger component and the dialog/menu it opens. Individual
  // handlers are tested in child specs; this suite pins the end-to-end
  // dispatch so a regression that disconnects state from its consumers is
  // caught even when both sides still pass in isolation.
  describe('Orchestration (M4–M6)', () => {
    // Each test renders the full Game container (DndContext + CardRegistry +
    // both player boards + preview panel) and drives MUI portal transitions
    // for Menu → Dialog flows. Cold jsdom render plus two portal transitions
    // routinely pushes a single test past vitest's 5s default under worker
    // contention. Every test in this block passes 15000ms explicitly as the
    // 3rd arg to `it(...)` — vi.setConfig in beforeAll/beforeEach didn't take
    // effect because the per-test timeout is captured at describe-registration
    // time, not at run time.
    const ORCHESTRATION_TIMEOUT_MS = 15000;

    it('Roll Die: TurnControls → RollDieDialog → rollDie dispatch', async () => {
      const webClient = createMockWebClient();
      renderWithProviders(<Game />, {
        preloadedState: buildGame({ localId: 1, opponentIds: [2] }),
        webClient,
      });

      fireEvent.click(screen.getByRole('button', { name: /roll die/i }));
      // Dialog opens via MUI portal+transition; await its inputs before
      // interacting to avoid flakes under worker contention.
      const sides = await screen.findByLabelText('Sides') as HTMLInputElement;
      const count = screen.getByLabelText('Count') as HTMLInputElement;
      fireEvent.change(sides, { target: { value: '20' } });
      fireEvent.change(count, { target: { value: '2' } });
      fireEvent.click(screen.getByRole('button', { name: /^roll$/i }));

      expect(webClient.request.game.rollDie).toHaveBeenCalledWith(1, { sides: 20, count: 2 });
    }, ORCHESTRATION_TIMEOUT_MS);

    it('Kick: TurnControls host menu → kickFromGame with chosen opponent', async () => {
      const webClient = createMockWebClient();
      renderWithProviders(<Game />, {
        // localId 1 is the host by fixture default (hostId: 1).
        preloadedState: buildGame({ localId: 1, opponentIds: [2, 3] }),
        webClient,
      });

      fireEvent.click(screen.getByRole('button', { name: /kick/i }));
      // P3 also appears in the OpponentSelector; pick the one inside the
      // MUI Menu popup. findAllByText waits for the portal to mount.
      const menuItem = (await screen.findAllByText('P3')).find((el) => el.closest('[role="menuitem"]'));
      fireEvent.click(menuItem!);

      expect(webClient.request.game.kickFromGame).toHaveBeenCalledWith(1, { playerId: 3 });
    }, ORCHESTRATION_TIMEOUT_MS);

    it('Create Token: PlayerContextMenu → CreateTokenDialog → createToken', async () => {
      const webClient = createMockWebClient();
      renderWithProviders(<Game />, {
        preloadedState: buildGame({ localId: 1, opponentIds: [2] }),
        webClient,
      });

      fireEvent.contextMenu(screen.getByTestId('player-info-1'));
      fireEvent.click(await screen.findByText('Create token…'));

      const nameInput = await screen.findByLabelText('Token name');
      fireEvent.change(nameInput, { target: { value: 'Goblin' } });
      fireEvent.click(screen.getByRole('button', { name: /^create$/i }));

      expect(webClient.request.game.createToken).toHaveBeenCalledWith(
        1,
        expect.objectContaining({ cardName: 'Goblin', zone: App.ZoneName.TABLE }),
      );
    }, ORCHESTRATION_TIMEOUT_MS);

    it('Mulligan same-size: HandContextMenu → mulligan with current hand count', async () => {
      const webClient = createMockWebClient();
      const state = buildGame({ localId: 1, opponentIds: [2] });
      // Seed the local hand with 5 cards so "same size" sends number: 5.
      const localPlayer = state.games.games[1].players[1];
      localPlayer.zones[App.ZoneName.HAND] = makeZoneEntry({
        name: App.ZoneName.HAND,
        cards: Array.from({ length: 5 }, (_, i) => makeCard({ id: 100 + i })),
        cardCount: 5,
      });
      renderWithProviders(<Game />, { preloadedState: state, webClient });

      fireEvent.contextMenu(screen.getByTestId('hand-zone'));
      fireEvent.click(await screen.findByText(/take mulligan \(same size\)/i));

      expect(webClient.request.game.mulligan).toHaveBeenCalledWith(1, { number: 5 });
    }, ORCHESTRATION_TIMEOUT_MS);

    it('Mulligan choose-size: negative input is translated to handSize + input', async () => {
      // Desktop's actMulligan (player_actions.cpp:308-354) treats 0 and
      // negative inputs as "relative to current hand size" before
      // dispatching Command_Mulligan. Regression guard for that convention.
      const webClient = createMockWebClient();
      const state = buildGame({ localId: 1, opponentIds: [2] });
      const localPlayer = state.games.games[1].players[1];
      localPlayer.zones[App.ZoneName.HAND] = makeZoneEntry({
        name: App.ZoneName.HAND,
        cards: Array.from({ length: 7 }, (_, i) => makeCard({ id: 100 + i })),
        cardCount: 7,
      });
      localPlayer.zones[App.ZoneName.DECK] = makeZoneEntry({
        name: App.ZoneName.DECK,
        cards: [],
        cardCount: 53,
      });
      renderWithProviders(<Game />, { preloadedState: state, webClient });

      fireEvent.contextMenu(screen.getByTestId('hand-zone'));
      fireEvent.click(await screen.findByText(/take mulligan \(choose size\)/i));

      // Helper text is visible to the user.
      expect(
        await screen.findByText('0 and lower are in comparison to current hand size.'),
      ).toBeInTheDocument();

      // Enter −1: server receives handSize + (−1) = 6.
      const input = screen.getByLabelText('New hand size');
      fireEvent.change(input, { target: { value: '-1' } });
      fireEvent.click(screen.getByRole('button', { name: /ok/i }));

      expect(webClient.request.game.mulligan).toHaveBeenCalledWith(1, { number: 6 });
    }, ORCHESTRATION_TIMEOUT_MS);

    it('Mulligan choose-size: positive integer passes through unchanged', async () => {
      const webClient = createMockWebClient();
      const state = buildGame({ localId: 1, opponentIds: [2] });
      const localPlayer = state.games.games[1].players[1];
      localPlayer.zones[App.ZoneName.HAND] = makeZoneEntry({
        name: App.ZoneName.HAND,
        cards: Array.from({ length: 7 }, (_, i) => makeCard({ id: 100 + i })),
        cardCount: 7,
      });
      localPlayer.zones[App.ZoneName.DECK] = makeZoneEntry({
        name: App.ZoneName.DECK,
        cards: [],
        cardCount: 53,
      });
      renderWithProviders(<Game />, { preloadedState: state, webClient });

      fireEvent.contextMenu(screen.getByTestId('hand-zone'));
      fireEvent.click(await screen.findByText(/take mulligan \(choose size\)/i));

      const input = await screen.findByLabelText('New hand size');
      fireEvent.change(input, { target: { value: '4' } });
      fireEvent.click(screen.getByRole('button', { name: /ok/i }));

      expect(webClient.request.game.mulligan).toHaveBeenCalledWith(1, { number: 4 });
    }, ORCHESTRATION_TIMEOUT_MS);

    it('Arrow-from-hand auto-plays the source card instead of sending a stale createArrow', async () => {
      // Desktop parity (card_item.cpp:243-250): dragging an arrow from a
      // local-hand card to a target outside the hand auto-plays the card.
      // The server re-keys the card id on the move, so sending createArrow
      // with the old hand cardId would be rejected. We resolve this as a
      // play-card intent and skip the arrow command.
      const webClient = createMockWebClient();
      const state = buildGame({
        localId: 1,
        opponentIds: [2],
        tableCards: [makeCard({ id: 50, name: 'Bear' })],
      });
      const localPlayer = state.games.games[1].players[1];
      localPlayer.zones[App.ZoneName.HAND] = makeZoneEntry({
        name: App.ZoneName.HAND,
        cards: [makeCard({ id: 10, name: 'Lightning Bolt' })],
        cardCount: 1,
      });
      renderWithProviders(<Game />, { preloadedState: state, webClient });

      // Right-click the hand card to open CardContextMenu.
      const handCard = document.querySelector('[data-card-zone="hand"][data-card-id="10"]')!;
      fireEvent.contextMenu(handCard);

      // MUI Menu transitions in — await its content before interacting.
      const drawArrowItem = await screen.findByText('Draw arrow from here');
      fireEvent.click(drawArrowItem);

      // Click a battlefield card — the handleCardClick path should detect
      // the hand-source + non-hand-target combo and dispatch moveCard.
      const tableCard = document.querySelector('[data-card-zone="table"][data-card-id="50"]')!;
      fireEvent.click(tableCard);

      await waitFor(() => {
        expect(webClient.request.game.moveCard).toHaveBeenCalledWith(
          1,
          expect.objectContaining({
            startPlayerId: 1,
            startZone: App.ZoneName.HAND,
            targetPlayerId: 1,
            targetZone: App.ZoneName.TABLE,
            cardsToMove: { card: [{ cardId: 10 }] },
          }),
        );
      });
      expect(webClient.request.game.createArrow).not.toHaveBeenCalled();
    }, ORCHESTRATION_TIMEOUT_MS);

    it('Mulligan choose-size: value outside [-handSize, handSize+deckSize] is rejected', async () => {
      const webClient = createMockWebClient();
      const state = buildGame({ localId: 1, opponentIds: [2] });
      const localPlayer = state.games.games[1].players[1];
      localPlayer.zones[App.ZoneName.HAND] = makeZoneEntry({
        name: App.ZoneName.HAND,
        cards: Array.from({ length: 7 }, (_, i) => makeCard({ id: 100 + i })),
        cardCount: 7,
      });
      localPlayer.zones[App.ZoneName.DECK] = makeZoneEntry({
        name: App.ZoneName.DECK,
        cards: [],
        cardCount: 53,
      });
      renderWithProviders(<Game />, { preloadedState: state, webClient });

      fireEvent.contextMenu(screen.getByTestId('hand-zone'));
      fireEvent.click(await screen.findByText(/take mulligan \(choose size\)/i));

      const input = await screen.findByLabelText('New hand size');
      fireEvent.change(input, { target: { value: '-99' } });
      fireEvent.click(screen.getByRole('button', { name: /ok/i }));

      expect(webClient.request.game.mulligan).not.toHaveBeenCalled();
      expect(screen.getByText(/between -7 and 60/i)).toBeInTheDocument();
    }, ORCHESTRATION_TIMEOUT_MS);

    it('Sideboard: PlayerContextMenu → SideboardDialog → setSideboardPlan with the accumulated moveList', async () => {
      const webClient = createMockWebClient();
      const state = buildGame({ localId: 1, opponentIds: [2] });
      // Seed the local deck + sideboard with distinct named cards.
      const localPlayer = state.games.games[1].players[1];
      localPlayer.zones[App.ZoneName.DECK] = makeZoneEntry({
        name: App.ZoneName.DECK,
        cards: [makeCard({ id: 100, name: 'Island' })],
        cardCount: 1,
      });
      localPlayer.zones[App.ZoneName.SIDEBOARD] = makeZoneEntry({
        name: App.ZoneName.SIDEBOARD,
        cards: [makeCard({ id: 200, name: 'Counterspell' })],
        cardCount: 1,
      });
      renderWithProviders(<Game />, { preloadedState: state, webClient });

      fireEvent.contextMenu(screen.getByTestId('player-info-1'));
      fireEvent.click(await screen.findByText(/view sideboard/i));
      fireEvent.click(await screen.findByRole('button', { name: /move Island to sideboard/i }));
      fireEvent.click(screen.getByRole('button', { name: /apply plan/i }));

      expect(webClient.request.game.setSideboardPlan).toHaveBeenCalledWith(
        1,
        expect.objectContaining({
          moveList: [
            { cardName: 'Island', startZone: App.ZoneName.DECK, targetZone: App.ZoneName.SIDEBOARD },
          ],
        }),
      );
    }, ORCHESTRATION_TIMEOUT_MS);

    it('Sideboard lock: toggling Lock sideboard dispatches setSideboardLock', async () => {
      const webClient = createMockWebClient();
      renderWithProviders(<Game />, {
        preloadedState: buildGame({ localId: 1, opponentIds: [2] }),
        webClient,
      });

      fireEvent.contextMenu(screen.getByTestId('player-info-1'));
      fireEvent.click(await screen.findByText(/view sideboard/i));
      fireEvent.click(await screen.findByLabelText('Lock sideboard'));

      expect(webClient.request.game.setSideboardLock).toHaveBeenCalledWith(1, { locked: true });
    }, ORCHESTRATION_TIMEOUT_MS);

    it('changeZoneProperties: toggling "Always reveal top card" on local deck dispatches the command', async () => {
      const webClient = createMockWebClient();
      renderWithProviders(<Game />, {
        preloadedState: buildGame({ localId: 1, opponentIds: [2] }),
        webClient,
      });

      fireEvent.contextMenu(
        screen
          .getByTestId('player-board-1')
          .querySelector(`[data-testid="zone-stack-${App.ZoneName.DECK}"]`)!,
      );
      fireEvent.click(await screen.findByText(/always reveal top card/i));

      expect(webClient.request.game.changeZoneProperties).toHaveBeenCalledWith(
        1,
        expect.objectContaining({
          zoneName: App.ZoneName.DECK,
          alwaysRevealTopCard: true,
        }),
      );
    }, ORCHESTRATION_TIMEOUT_MS);
  });
});
