import { fireEvent, screen } from '@testing-library/react';
import { create } from '@bufbuild/protobuf';
import {
  renderWithProviders,
  makeStoreState,
  makeUser,
  connectedWithRoomsState,
} from '../../../__test-utils__';
import { App, Data } from '@app/types';
import { GameTypes } from '@app/store';
import GameSelector from './GameSelector';

const { mockUseWebClient, mockNavigate } = vi.hoisted(() => ({
  mockUseWebClient: vi.fn(),
  mockNavigate: vi.fn(),
}));
vi.mock('@app/hooks', async (importOriginal) => {
  const actual = await importOriginal<typeof import('@app/hooks')>();
  return { ...actual, useWebClient: mockUseWebClient };
});
vi.mock('react-router-dom', async (importOriginal) => {
  const actual = await importOriginal<typeof import('react-router-dom')>();
  return { ...actual, useNavigate: () => mockNavigate };
});

function makeRoomEntry(games: Data.ServerInfo_Game[] = [], gametypeMap: Record<number, string> = {}) {
  return {
    info: create(Data.ServerInfo_RoomSchema, { roomId: 1, name: 'Main' }),
    gametypeMap,
    order: 0,
    games: Object.fromEntries(games.map((info) => [info.gameId, { info, gameType: '' }])),
    users: {},
  };
}

function makeGame(overrides: any = {}): Data.ServerInfo_Game {
  return create(Data.ServerInfo_GameSchema, {
    gameId: 1,
    roomId: 1,
    description: 'Test',
    maxPlayers: 4,
    playerCount: 1,
    spectatorsAllowed: true,
    ...overrides,
  });
}

function makeWebClient() {
  return {
    request: {
      rooms: {
        joinRoom: vi.fn(),
        leaveRoom: vi.fn(),
        roomSay: vi.fn(),
        createGame: vi.fn(),
        joinGame: vi.fn(),
      },
    },
  } as any;
}

function buildState(
  room: ReturnType<typeof makeRoomEntry>,
  user = makeUser(),
  selectedGameId?: number,
  roomsOverrides: Partial<{ joinGamePending: boolean; joinGameError: { code: number; message: string } | null }> = {},
) {
  return makeStoreState({
    ...connectedWithRoomsState,
    rooms: {
      rooms: { 1: room },
      joinedRoomIds: { 1: true },
      joinedGameIds: {},
      messages: { 1: [] },
      sortGamesBy: { field: App.GameSortField.START_TIME, order: App.SortDirection.DESC },
      sortUsersBy: { field: App.UserSortField.NAME, order: App.SortDirection.ASC },
      selectedGameIds: selectedGameId != null ? { 1: selectedGameId } : {},
      gameFilters: {},
      joinGamePending: false,
      joinGameError: null,
      ...roomsOverrides,
    } as any,
    server: {
      ...(connectedWithRoomsState.server as any),
      user,
    } as any,
  });
}

beforeEach(() => {
  mockUseWebClient.mockReset();
  mockNavigate.mockReset();
});

describe('GameSelector', () => {
  it('renders the count header from getRoomGameCounts', () => {
    mockUseWebClient.mockReturnValue(makeWebClient());
    const room = makeRoomEntry([makeGame({ gameId: 1 }), makeGame({ gameId: 2 })]);
    renderWithProviders(<GameSelector room={room as any} />, { preloadedState: buildState(room) });
    expect(screen.getByText('Games shown: 2 / 2')).toBeInTheDocument();
  });

  it('Join button is disabled until a game is selected', () => {
    mockUseWebClient.mockReturnValue(makeWebClient());
    const room = makeRoomEntry([makeGame({ gameId: 1 })]);
    renderWithProviders(<GameSelector room={room as any} />, { preloadedState: buildState(room) });
    expect(screen.getByRole('button', { name: /^Join$/ })).toBeDisabled();
  });

  it('Join button enabled and dispatches joinGame when a game is selected', () => {
    const client = makeWebClient();
    mockUseWebClient.mockReturnValue(client);
    const game = makeGame({ gameId: 7, withPassword: false });
    const room = makeRoomEntry([game]);
    renderWithProviders(<GameSelector room={room as any} />, {
      preloadedState: buildState(room, makeUser(), 7),
    });

    const joinBtn = screen.getByRole('button', { name: /^Join$/ });
    expect(joinBtn).not.toBeDisabled();
    fireEvent.click(joinBtn);

    expect(client.request.rooms.joinGame).toHaveBeenCalledTimes(1);
    expect(client.request.rooms.joinGame).toHaveBeenCalledWith(1, expect.objectContaining({
      gameId: 7,
      spectator: false,
      joinAsJudge: false,
      password: '',
    }));
  });

  it('clicking Join on a password-protected game opens the password prompt before sending', () => {
    const client = makeWebClient();
    mockUseWebClient.mockReturnValue(client);
    const game = makeGame({ gameId: 8, withPassword: true });
    const room = makeRoomEntry([game]);
    renderWithProviders(<GameSelector room={room as any} />, {
      preloadedState: buildState(room, makeUser(), 8),
    });

    fireEvent.click(screen.getByRole('button', { name: /^Join$/ }));

    expect(client.request.rooms.joinGame).not.toHaveBeenCalled();
    expect(screen.getByText('Password required')).toBeInTheDocument();

    fireEvent.change(screen.getByLabelText('Password'), { target: { value: 'hunter2' } });
    // The dialog has its own "Join" button — find both and click the last (in dialog).
    const joinButtons = screen.getAllByRole('button', { name: /^Join$/ });
    fireEvent.click(joinButtons[joinButtons.length - 1]);

    expect(client.request.rooms.joinGame).toHaveBeenCalledTimes(1);
    expect(client.request.rooms.joinGame).toHaveBeenCalledWith(1, expect.objectContaining({
      gameId: 8,
      password: 'hunter2',
    }));
  });

  it('Spectate button is disabled when spectators are not allowed', () => {
    mockUseWebClient.mockReturnValue(makeWebClient());
    const game = makeGame({ gameId: 1, spectatorsAllowed: false });
    const room = makeRoomEntry([game]);
    renderWithProviders(<GameSelector room={room as any} />, {
      preloadedState: buildState(room, makeUser(), 1),
    });
    expect(screen.getByRole('button', { name: /Join as Spectator/i })).toBeDisabled();
  });

  it('Join is disabled when the selected game is full', () => {
    mockUseWebClient.mockReturnValue(makeWebClient());
    const game = makeGame({ gameId: 1, playerCount: 4, maxPlayers: 4 });
    const room = makeRoomEntry([game]);
    renderWithProviders(<GameSelector room={room as any} />, {
      preloadedState: buildState(room, makeUser(), 1),
    });
    expect(screen.getByRole('button', { name: /^Join$/ })).toBeDisabled();
  });

  it('judge buttons are hidden when the user is not a judge', () => {
    mockUseWebClient.mockReturnValue(makeWebClient());
    const room = makeRoomEntry([]);
    renderWithProviders(<GameSelector room={room as any} />, {
      preloadedState: buildState(room, makeUser({ userLevel: 0 })),
    });
    expect(screen.queryByRole('button', { name: /Join as Judge$/i })).not.toBeInTheDocument();
  });

  it('judge buttons are visible when the user has the IsJudge flag', () => {
    mockUseWebClient.mockReturnValue(makeWebClient());
    const room = makeRoomEntry([]);
    renderWithProviders(<GameSelector room={room as any} />, {
      preloadedState: buildState(room, makeUser({ userLevel: Data.ServerInfo_User_UserLevelFlag.IsJudge })),
    });
    expect(screen.getByRole('button', { name: /Join as Judge$/i })).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /Join as Judge Spectator/i })).toBeInTheDocument();
  });

  it('renders AlertDialog with the join error message from state', () => {
    mockUseWebClient.mockReturnValue(makeWebClient());
    const room = makeRoomEntry([]);
    renderWithProviders(<GameSelector room={room as any} />, {
      preloadedState: buildState(room, makeUser(), undefined, {
        joinGameError: { code: 10, message: 'The game is already full.' },
      }),
    });
    expect(screen.getByText('Error')).toBeInTheDocument();
    expect(screen.getByText('The game is already full.')).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /^ok$/i })).toBeInTheDocument();
  });

  it('does not render AlertDialog when joinGameError is null (covers silent RespContextError)', () => {
    mockUseWebClient.mockReturnValue(makeWebClient());
    const room = makeRoomEntry([]);
    renderWithProviders(<GameSelector room={room as any} />, {
      preloadedState: buildState(room, makeUser(), undefined, { joinGameError: null }),
    });
    // Only the CreateGame / FilterGames / PromptDialog / AlertDialog dialogs might exist; none
    // should be open, so no role="dialog" in the DOM.
    expect(screen.queryByRole('dialog')).not.toBeInTheDocument();
  });

  it('clicking Join on a game already present in games.games navigates to /game without sending a command', () => {
    const client = makeWebClient();
    mockUseWebClient.mockReturnValue(client);
    const game = makeGame({ gameId: 7, withPassword: false });
    const room = makeRoomEntry([game]);
    const state = buildState(room, makeUser(), 7);
    (state as any).games = { games: { 7: { info: { gameId: 7 } } } };
    renderWithProviders(<GameSelector room={room as any} />, { preloadedState: state });

    fireEvent.click(screen.getByRole('button', { name: /^Join$/ }));

    expect(client.request.rooms.joinGame).not.toHaveBeenCalled();
    expect(mockNavigate).toHaveBeenCalledWith(App.RouteEnum.GAME);
  });

  it('dispatching GAME_JOINED navigates to /game (mirrors JOIN_ROOM → /room)', async () => {
    mockUseWebClient.mockReturnValue(makeWebClient());
    const room = makeRoomEntry([]);
    const { store } = renderWithProviders(<GameSelector room={room as any} />, {
      preloadedState: buildState(room),
    });

    mockNavigate.mockClear();
    store.dispatch({
      type: GameTypes.GAME_JOINED,
      payload: { data: { gameInfo: { gameId: 42 }, hostId: 0, playerId: 0, spectator: false } },
    });

    await Promise.resolve();
    expect(mockNavigate).toHaveBeenCalledWith(App.RouteEnum.GAME);
  });

  it('Join button is disabled while joinGamePending is true even when a game is selected', () => {
    mockUseWebClient.mockReturnValue(makeWebClient());
    const game = makeGame({ gameId: 7 });
    const room = makeRoomEntry([game]);
    renderWithProviders(<GameSelector room={room as any} />, {
      preloadedState: buildState(room, makeUser(), 7, { joinGamePending: true }),
    });
    expect(screen.getByRole('button', { name: /^Join$/ })).toBeDisabled();
    expect(screen.getByRole('button', { name: /Join as Spectator/i })).toBeDisabled();
  });

  it('clicking Create then submitting forwards createGame', () => {
    const client = makeWebClient();
    mockUseWebClient.mockReturnValue(client);
    const room = makeRoomEntry([]);
    renderWithProviders(<GameSelector room={room as any} />, { preloadedState: buildState(room) });
    fireEvent.click(screen.getByRole('button', { name: /^Create$/ }));
    // The dialog Submit button is the second matching "Create" button
    const createButtons = screen.getAllByRole('button', { name: /^Create$/ });
    fireEvent.click(createButtons[createButtons.length - 1]);
    expect(client.request.rooms.createGame).toHaveBeenCalledTimes(1);
    expect(client.request.rooms.createGame.mock.calls[0][0]).toBe(1);
  });
});
