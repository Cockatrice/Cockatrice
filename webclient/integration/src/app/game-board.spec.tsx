// Exercises the full Game container under the real Redux store + real
// reducers + real React chain. We dispatch game lifecycle events via
// GameDispatch (the same path real event handlers take) and assert the
// Game container's UI tracks state transitions.

import { act, fireEvent, waitFor, screen, within } from '@testing-library/react';
import { create } from '@bufbuild/protobuf';
import { useLocation } from 'react-router-dom';
import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';

import { Data } from '@app/types';
import { Command_GameSay_ext } from '@app/generated';
import { GameDispatch, ServerDispatch, store } from '@app/store';
import { WebsocketTypes } from '@app/websocket/types';

import Game from '../../../src/containers/Game/Game';
import { renderAppScreen } from './helpers';
import { findLastGameCommand } from '../helpers/command-capture';
import { connectRaw } from '../helpers/setup';

// Surfaces the current MemoryRouter pathname so navigate() side-effects
// (e.g. useGameLifecycle → /server on kick) can be asserted. Depends on
// `renderAppScreen` → `renderWithProviders` wrapping its tree in a
// `MemoryRouter`; if that harness ever moves the probe outside a Router,
// `useLocation()` will throw "useLocation() may be used only in the
// context of a <Router> component." — fix by adding an inline
// `<MemoryRouter>` around the probe OR by teaching the harness about it.
function LocationProbe() {
  const location = useLocation();
  return <span data-testid="app-location">{location.pathname}</span>;
}

function buildEventGameJoined(args: {
  gameId: number;
  localPlayerId: number;
  hostId: number;
}): Data.Event_GameJoined {
  return create(Data.Event_GameJoinedSchema, {
    gameInfo: create(Data.ServerInfo_GameSchema, {
      gameId: args.gameId,
      roomId: 1,
      description: 'Integration Test Game',
      gameTypes: [],
      started: false,
    }),
    hostId: args.hostId,
    playerId: args.localPlayerId,
    spectator: false,
    judge: false,
    resuming: false,
  });
}

function buildEventGameStateChanged(
  playerIds: number[],
  localId: number,
): Data.Event_GameStateChanged {
  return create(Data.Event_GameStateChangedSchema, {
    gameStarted: true,
    activePlayerId: localId,
    activePhase: 0,
    playerList: playerIds.map((pid) =>
      create(Data.ServerInfo_PlayerSchema, {
        properties: create(Data.ServerInfo_PlayerPropertiesSchema, {
          playerId: pid,
          userInfo: create(Data.ServerInfo_UserSchema, { name: `P${pid}` }),
          spectator: false,
          conceded: false,
          readyStart: false,
          judge: false,
        }),
        deckList: '',
        zoneList: [
          create(Data.ServerInfo_ZoneSchema, {
            name: 'table',
            type: 1,
            withCoords: true,
            cardCount: 0,
            cardList: [],
          }),
          create(Data.ServerInfo_ZoneSchema, {
            name: 'hand',
            type: 0,
            withCoords: false,
            cardCount: 0,
            cardList: [],
          }),
          create(Data.ServerInfo_ZoneSchema, {
            name: 'deck',
            type: 2,
            withCoords: false,
            cardCount: 40,
            cardList: [],
          }),
          create(Data.ServerInfo_ZoneSchema, {
            name: 'grave',
            type: 1,
            withCoords: false,
            cardCount: 0,
            cardList: [],
          }),
          create(Data.ServerInfo_ZoneSchema, {
            name: 'rfg',
            type: 1,
            withCoords: false,
            cardCount: 0,
            cardList: [],
          }),
        ],
        counterList: [],
        arrowList: [],
      }),
    ),
  });
}

function simulateConnected() {
  act(() => {
    ServerDispatch.updateStatus(WebsocketTypes.StatusEnum.LOGGED_IN, null);
  });
}

afterEach(() => {
  act(() => {
    for (const gameId of Object.keys(store.getState().games.games)) {
      GameDispatch.gameLeft(Number(gameId));
    }
    ServerDispatch.updateStatus(WebsocketTypes.StatusEnum.DISCONNECTED, null);
  });
});

beforeEach(() => {
  // Integration setup installs fake timers for KeepAliveService control;
  // waitFor / React effects need real timers to run between dispatch and assert.
  vi.useRealTimers();
  simulateConnected();
});

describe('Game board integration', () => {
  it('renders the empty-board placeholder until a game is joined', () => {
    renderAppScreen(<Game />);

    expect(screen.getByTestId('game-empty')).toBeInTheDocument();
    expect(screen.getByTestId('phase-bar')).toBeInTheDocument();
    expect(screen.getByTestId('right-panel')).toBeInTheDocument();
  });

  it('transitions from empty → active board when gameJoined + gameStateChanged fire', async () => {
    renderAppScreen(<Game />);

    expect(screen.getByTestId('game-empty')).toBeInTheDocument();

    act(() => {
      GameDispatch.gameJoined(
        buildEventGameJoined({ gameId: 42, localPlayerId: 1, hostId: 1 }),
      );
      GameDispatch.gameStateChanged(
        42,
        buildEventGameStateChanged([1, 2], 1),
      );
    });

    await waitFor(() => {
      expect(screen.queryByTestId('game-empty')).not.toBeInTheDocument();
    });

    expect(screen.getByTestId('player-board-1')).toBeInTheDocument();
    expect(screen.getByTestId('player-board-2')).toBeInTheDocument();
    expect(screen.getByTestId('hand-zone')).toBeInTheDocument();
  });

  it('returns to the empty placeholder when gameLeft fires', async () => {
    renderAppScreen(<Game />);

    act(() => {
      GameDispatch.gameJoined(
        buildEventGameJoined({ gameId: 42, localPlayerId: 1, hostId: 1 }),
      );
      GameDispatch.gameStateChanged(
        42,
        buildEventGameStateChanged([1, 2], 1),
      );
    });

    await waitFor(() => {
      expect(screen.getByTestId('player-board-1')).toBeInTheDocument();
    });

    act(() => {
      GameDispatch.gameLeft(42);
    });

    await waitFor(() => {
      expect(screen.getByTestId('game-empty')).toBeInTheDocument();
    });

    expect(screen.queryByTestId('player-board-1')).not.toBeInTheDocument();
  });

  it('hides the opponent selector for 2-player games but shows it for 3+', async () => {
    renderAppScreen(<Game />);

    act(() => {
      GameDispatch.gameJoined(
        buildEventGameJoined({ gameId: 42, localPlayerId: 1, hostId: 1 }),
      );
      GameDispatch.gameStateChanged(
        42,
        buildEventGameStateChanged([1, 2], 1),
      );
    });

    await waitFor(() => {
      expect(screen.getByTestId('player-board-1')).toBeInTheDocument();
    });

    expect(screen.queryByTestId('opponent-selector')).not.toBeInTheDocument();

    act(() => {
      GameDispatch.gameStateChanged(
        42,
        buildEventGameStateChanged([1, 2, 3], 1),
      );
    });

    await waitFor(() => {
      expect(screen.getByTestId('opponent-selector')).toBeInTheDocument();
    });
  });

  it('mirrors the opponent board and leaves the local board upright', async () => {
    renderAppScreen(<Game />);

    act(() => {
      GameDispatch.gameJoined(
        buildEventGameJoined({ gameId: 42, localPlayerId: 1, hostId: 1 }),
      );
      GameDispatch.gameStateChanged(
        42,
        buildEventGameStateChanged([1, 2], 1),
      );
    });

    await waitFor(() => {
      expect(screen.getByTestId('player-board-2')).toHaveClass('player-board--mirrored');
    });

    expect(screen.getByTestId('player-board-1')).not.toHaveClass('player-board--mirrored');
  });

  it('renders the deck/graveyard/exile rail in desktop order (no stack in rail)', async () => {
    renderAppScreen(<Game />);

    act(() => {
      GameDispatch.gameJoined(
        buildEventGameJoined({ gameId: 42, localPlayerId: 1, hostId: 1 }),
      );
      GameDispatch.gameStateChanged(
        42,
        buildEventGameStateChanged([1, 2], 1),
      );
    });

    await waitFor(() => {
      expect(screen.getByTestId('player-board-1')).toBeInTheDocument();
    });

    const localBoard = screen.getByTestId('player-board-1');
    const rail = within(localBoard).getByTestId('zone-rail');
    const labels = Array.from(rail.querySelectorAll('.zone-stack__label')).map(
      (n) => n.textContent,
    );
    expect(labels).toEqual(['Deck', 'Graveyard', 'Exile']);
    expect(within(rail).queryByText('Stack')).not.toBeInTheDocument();
  });

  it('sends a game_say command through the socket when a chat message is submitted', async () => {
    // Establish a real mock socket so the outbound CommandContainer is captured.
    connectRaw();

    renderAppScreen(<Game />);

    act(() => {
      GameDispatch.gameJoined(
        buildEventGameJoined({ gameId: 42, localPlayerId: 1, hostId: 1 }),
      );
      // buildEventGameStateChanged sets gameStarted: true, suppressing the
      // deck-select dialog which would otherwise block focus/interaction.
      GameDispatch.gameStateChanged(
        42,
        buildEventGameStateChanged([1, 2], 1),
      );
    });

    await waitFor(() => {
      expect(screen.getByLabelText('game chat input')).not.toBeDisabled();
    });

    const input = screen.getByLabelText('game chat input') as HTMLInputElement;
    fireEvent.change(input, { target: { value: 'gl hf' } });
    fireEvent.submit(input.closest('form')!);

    const captured = findLastGameCommand(Command_GameSay_ext);
    expect(captured.value.message).toBe('gl hf');
    expect(captured.gameId).toBe(42);
  });

  it('navigates to /server when the local user is kicked', async () => {
    renderAppScreen(
      <>
        <Game />
        <LocationProbe />
      </>,
    );

    act(() => {
      GameDispatch.gameJoined(
        buildEventGameJoined({ gameId: 42, localPlayerId: 1, hostId: 1 }),
      );
      GameDispatch.gameStateChanged(
        42,
        buildEventGameStateChanged([1, 2], 1),
      );
    });

    await waitFor(() => {
      expect(screen.getByTestId('player-board-1')).toBeInTheDocument();
    });

    act(() => {
      GameDispatch.kicked(42);
    });

    await waitFor(() => {
      expect(screen.getByTestId('app-location')).toHaveTextContent('/server');
    });
  });

  it('navigates to /server when the game is closed by the host', async () => {
    renderAppScreen(
      <>
        <Game />
        <LocationProbe />
      </>,
    );

    act(() => {
      GameDispatch.gameJoined(
        buildEventGameJoined({ gameId: 42, localPlayerId: 1, hostId: 1 }),
      );
      GameDispatch.gameStateChanged(
        42,
        buildEventGameStateChanged([1, 2], 1),
      );
    });

    await waitFor(() => {
      expect(screen.getByTestId('player-board-1')).toBeInTheDocument();
    });

    act(() => {
      GameDispatch.gameClosed(42);
    });

    await waitFor(() => {
      expect(screen.getByTestId('app-location')).toHaveTextContent('/server');
    });
  });

  it('reflects a host change through both PlayerList badge and PlayerInfoPanel', async () => {
    renderAppScreen(<Game />);

    act(() => {
      GameDispatch.gameJoined(
        buildEventGameJoined({ gameId: 42, localPlayerId: 1, hostId: 1 }),
      );
      GameDispatch.gameStateChanged(
        42,
        buildEventGameStateChanged([1, 2], 1),
      );
    });

    await waitFor(() => {
      expect(screen.getByTestId('player-list-item-1')).toBeInTheDocument();
    });

    // Host starts as 1; badge should be on row 1.
    expect(
      screen.getByTestId('player-list-item-1').querySelector('.player-list__host-badge'),
    ).not.toBeNull();
    expect(
      screen.getByTestId('player-list-item-2').querySelector('.player-list__host-badge'),
    ).toBeNull();

    // Host changes to player 2.
    act(() => {
      GameDispatch.gameHostChanged(42, 2);
    });

    await waitFor(() => {
      expect(
        screen.getByTestId('player-list-item-2').querySelector('.player-list__host-badge'),
      ).not.toBeNull();
    });
    expect(
      screen.getByTestId('player-list-item-1').querySelector('.player-list__host-badge'),
    ).toBeNull();
  });

  it('auto-opens the DeckSelectDialog when a game is joined and not started', async () => {
    renderAppScreen(<Game />);

    act(() => {
      GameDispatch.gameJoined(
        buildEventGameJoined({ gameId: 42, localPlayerId: 1, hostId: 1 }),
      );
      GameDispatch.gameStateChanged(
        42,
        create(Data.Event_GameStateChangedSchema, {
          gameStarted: false,
          activePlayerId: 1,
          activePhase: -1,
          playerList: [1, 2].map((pid) =>
            create(Data.ServerInfo_PlayerSchema, {
              properties: create(Data.ServerInfo_PlayerPropertiesSchema, {
                playerId: pid,
                userInfo: create(Data.ServerInfo_UserSchema, { name: `P${pid}` }),
              }),
              deckList: '',
              zoneList: [],
              counterList: [],
              arrowList: [],
            }),
          ),
        }),
      );
    });

    await waitFor(() => {
      expect(screen.getByLabelText('deck list')).toBeInTheDocument();
    });
  });
});
