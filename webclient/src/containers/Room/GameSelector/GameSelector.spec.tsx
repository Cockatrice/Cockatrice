import { fireEvent, screen } from '@testing-library/react';
import { create } from '@bufbuild/protobuf';
import {
  renderWithProviders,
  makeStoreState,
  makeUser,
  connectedWithRoomsState,
} from '../../../__test-utils__';
import { App, Data } from '@app/types';
import GameSelector from './GameSelector';

const { mockUseWebClient } = vi.hoisted(() => ({ mockUseWebClient: vi.fn() }));
vi.mock('@app/hooks', async (importOriginal) => {
  const actual = await importOriginal<typeof import('@app/hooks')>();
  return { ...actual, useWebClient: mockUseWebClient };
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
    } as any,
    server: {
      ...(connectedWithRoomsState.server as any),
      user,
    } as any,
  });
}

beforeEach(() => {
  mockUseWebClient.mockReset();
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
