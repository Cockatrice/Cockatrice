// Hoisted so the mockDispatch reference is available inside the vi.mock factory
// below and can be asserted against directly from each test.
const { mockDispatch } = vi.hoisted(() => ({ mockDispatch: vi.fn() }));
vi.mock('..', () => ({ store: { dispatch: mockDispatch } }));

import { Actions } from './rooms.actions';
import { Dispatch } from './rooms.dispatch';
import { DEFAULT_GAME_FILTERS } from './gameFilters';
import { makeGame, makeMessage, makeRoom, makeUser } from './__mocks__/rooms-fixtures';
import { App } from '@app/types';

beforeEach(() => {
  mockDispatch.mockClear();
});

describe('Dispatch', () => {
  it('clearStore dispatches Actions.clearStore()', () => {
    Dispatch.clearStore();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.clearStore());
  });

  it('updateRooms dispatches Actions.updateRooms()', () => {
    const rooms = [makeRoom()];
    Dispatch.updateRooms(rooms);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.updateRooms({ rooms }));
  });

  it('joinRoom dispatches Actions.joinRoom()', () => {
    const roomInfo = makeRoom({ roomId: 2 });
    Dispatch.joinRoom(roomInfo);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.joinRoom({ roomInfo }));
  });

  it('leaveRoom dispatches Actions.leaveRoom()', () => {
    Dispatch.leaveRoom(3);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.leaveRoom({ roomId: 3 }));
  });

  it('addMessage with message.name falsy → dispatches only Actions.addMessage()', () => {
    const message = { ...makeMessage(), name: undefined };
    Dispatch.addMessage(1, message);
    expect(mockDispatch).toHaveBeenCalledTimes(1);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.addMessage({ roomId: 1, message }));
  });

  it('addMessage with message.name truthy → dispatches Actions.addMessage()', () => {
    const message = { ...makeMessage(), name: 'Alice' };
    Dispatch.addMessage(1, message);
    expect(mockDispatch).toHaveBeenCalledTimes(1);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.addMessage({ roomId: 1, message }));
  });

  it('updateGames dispatches Actions.updateGames()', () => {
    const games = [makeGame()];
    Dispatch.updateGames(1, games);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.updateGames({ roomId: 1, games }));
  });

  it('userJoined dispatches Actions.userJoined()', () => {
    const user = makeUser();
    Dispatch.userJoined(1, user);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.userJoined({ roomId: 1, user }));
  });

  it('userLeft dispatches Actions.userLeft()', () => {
    Dispatch.userLeft(1, 'Alice');
    expect(mockDispatch).toHaveBeenCalledWith(Actions.userLeft({ roomId: 1, name: 'Alice' }));
  });

  it('sortGames dispatches Actions.sortGames()', () => {
    Dispatch.sortGames(1, App.GameSortField.START_TIME, App.SortDirection.ASC);
    expect(mockDispatch).toHaveBeenCalledWith(
      Actions.sortGames({ roomId: 1, field: App.GameSortField.START_TIME, order: App.SortDirection.ASC })
    );
  });

  it('removeMessages dispatches Actions.removeMessages()', () => {
    Dispatch.removeMessages(1, 'Alice', 5);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.removeMessages({ roomId: 1, name: 'Alice', amount: 5 }));
  });

  it('gameCreated dispatches Actions.gameCreated()', () => {
    Dispatch.gameCreated(2);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.gameCreated({ roomId: 2 }));
  });

  it('joinedGame dispatches Actions.joinedGame()', () => {
    Dispatch.joinedGame(1, 5);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.joinedGame({ roomId: 1, gameId: 5 }));
  });

  it('selectGame dispatches Actions.selectGame()', () => {
    Dispatch.selectGame(1, 7);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.selectGame({ roomId: 1, gameId: 7 }));
  });

  it('selectGame with undefined dispatches a clear', () => {
    Dispatch.selectGame(1, undefined);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.selectGame({ roomId: 1, gameId: undefined }));
  });

  it('setGameFilters dispatches Actions.setGameFilters()', () => {
    const filters = { ...DEFAULT_GAME_FILTERS, hideFullGames: true };
    Dispatch.setGameFilters(1, filters);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.setGameFilters({ roomId: 1, filters }));
  });

  it('clearGameFilters dispatches Actions.clearGameFilters()', () => {
    Dispatch.clearGameFilters(1);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.clearGameFilters({ roomId: 1 }));
  });

  it('setJoinGamePending dispatches Actions.setJoinGamePending()', () => {
    Dispatch.setJoinGamePending(true);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.setJoinGamePending({ pending: true }));
  });

  it('setJoinGameError dispatches Actions.setJoinGameError()', () => {
    Dispatch.setJoinGameError(10, 'The game is already full.');
    expect(mockDispatch).toHaveBeenCalledWith(
      Actions.setJoinGameError({ code: 10, message: 'The game is already full.' })
    );
  });

  it('clearJoinGameError dispatches Actions.clearJoinGameError()', () => {
    Dispatch.clearJoinGameError();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.clearJoinGameError());
  });
});
