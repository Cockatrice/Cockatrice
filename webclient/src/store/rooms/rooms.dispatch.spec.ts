jest.mock('store/store', () => ({ store: { dispatch: jest.fn() } }));
jest.mock('redux-form', () => ({
  reset: jest.fn((form) => ({ type: '@@redux-form/RESET', meta: { form } })),
}));

import { store } from 'store/store';
import { reset } from 'redux-form';
import { Actions } from './rooms.actions';
import { Dispatch } from './rooms.dispatch';
import { makeGame, makeMessage, makeRoom, makeUser } from './__mocks__/rooms-fixtures';
import { GameSortField, SortDirection } from 'types';

beforeEach(() => jest.clearAllMocks());

describe('Dispatch', () => {
  it('clearStore dispatches Actions.clearStore()', () => {
    Dispatch.clearStore();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.clearStore());
  });

  it('updateRooms dispatches Actions.updateRooms()', () => {
    const rooms = [makeRoom()];
    Dispatch.updateRooms(rooms);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.updateRooms(rooms));
  });

  it('joinRoom dispatches Actions.joinRoom()', () => {
    const roomInfo = makeRoom({ roomId: 2 });
    Dispatch.joinRoom(roomInfo);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.joinRoom(roomInfo));
  });

  it('leaveRoom dispatches Actions.leaveRoom()', () => {
    Dispatch.leaveRoom(3);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.leaveRoom(3));
  });

  it('addMessage with message.name falsy → dispatches only Actions.addMessage()', () => {
    const message = { ...makeMessage(), name: undefined };
    Dispatch.addMessage(1, message);
    expect(store.dispatch).toHaveBeenCalledTimes(1);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.addMessage(1, message));
  });

  it('addMessage with message.name truthy → dispatches reset("sayMessage") then Actions.addMessage()', () => {
    const message = { ...makeMessage(), name: 'Alice' };
    Dispatch.addMessage(1, message);
    expect(store.dispatch).toHaveBeenNthCalledWith(1, (reset as jest.Mock)('sayMessage'));
    expect(store.dispatch).toHaveBeenNthCalledWith(2, Actions.addMessage(1, message));
  });

  it('updateGames dispatches Actions.updateGames()', () => {
    const games = [makeGame()];
    Dispatch.updateGames(1, games);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.updateGames(1, games));
  });

  it('userJoined dispatches Actions.userJoined()', () => {
    const user = makeUser();
    Dispatch.userJoined(1, user);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.userJoined(1, user));
  });

  it('userLeft dispatches Actions.userLeft()', () => {
    Dispatch.userLeft(1, 'Alice');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.userLeft(1, 'Alice'));
  });

  it('sortGames dispatches Actions.sortGames()', () => {
    Dispatch.sortGames(1, GameSortField.START_TIME, SortDirection.ASC);
    expect(store.dispatch).toHaveBeenCalledWith(
      Actions.sortGames(1, GameSortField.START_TIME, SortDirection.ASC)
    );
  });

  it('removeMessages dispatches Actions.removeMessages()', () => {
    Dispatch.removeMessages(1, 'Alice', 5);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.removeMessages(1, 'Alice', 5));
  });

  it('gameCreated dispatches Actions.gameCreated()', () => {
    Dispatch.gameCreated(2);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.gameCreated(2));
  });

  it('joinedGame dispatches Actions.joinedGame()', () => {
    Dispatch.joinedGame(1, 5);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.joinedGame(1, 5));
  });
});
