import { Actions } from './rooms.actions';
import { Types } from './rooms.types';
import { makeGame, makeMessage, makeRoom, makeUser } from './__mocks__/rooms-fixtures';
import { App } from '@app/types';

describe('Actions', () => {
  it('clearStore', () => {
    expect(Actions.clearStore()).toEqual({ type: Types.CLEAR_STORE, payload: undefined });
  });

  it('updateRooms', () => {
    const rooms = [makeRoom()];
    expect(Actions.updateRooms({ rooms })).toEqual({ type: Types.UPDATE_ROOMS, payload: { rooms } });
  });

  it('joinRoom', () => {
    const roomInfo = makeRoom({ roomId: 2 });
    expect(Actions.joinRoom({ roomInfo })).toEqual({ type: Types.JOIN_ROOM, payload: { roomInfo } });
  });

  it('leaveRoom', () => {
    expect(Actions.leaveRoom({ roomId: 3 })).toEqual({ type: Types.LEAVE_ROOM, payload: { roomId: 3 } });
  });

  it('addMessage', () => {
    const message = makeMessage();
    expect(Actions.addMessage({ roomId: 1, message })).toEqual({ type: Types.ADD_MESSAGE, payload: { roomId: 1, message } });
  });

  it('updateGames', () => {
    const games = [makeGame()];
    expect(Actions.updateGames({ roomId: 1, games })).toEqual({ type: Types.UPDATE_GAMES, payload: { roomId: 1, games } });
  });

  it('userJoined', () => {
    const user = makeUser();
    expect(Actions.userJoined({ roomId: 1, user })).toEqual({ type: Types.USER_JOINED, payload: { roomId: 1, user } });
  });

  it('userLeft', () => {
    expect(Actions.userLeft({ roomId: 1, name: 'Alice' })).toEqual({ type: Types.USER_LEFT, payload: { roomId: 1, name: 'Alice' } });
  });

  it('sortGames', () => {
    expect(Actions.sortGames({ roomId: 1, field: App.GameSortField.START_TIME, order: App.SortDirection.ASC })).toEqual({
      type: Types.SORT_GAMES,
      payload: {
        roomId: 1,
        field: App.GameSortField.START_TIME,
        order: App.SortDirection.ASC,
      },
    });
  });

  it('removeMessages', () => {
    expect(Actions.removeMessages({ roomId: 1, name: 'Alice', amount: 3 })).toEqual({
      type: Types.REMOVE_MESSAGES,
      payload: {
        roomId: 1,
        name: 'Alice',
        amount: 3,
      },
    });
  });

  it('gameCreated', () => {
    expect(Actions.gameCreated({ roomId: 2 })).toEqual({ type: Types.GAME_CREATED, payload: { roomId: 2 } });
  });

  it('joinedGame', () => {
    expect(Actions.joinedGame({ roomId: 1, gameId: 5 })).toEqual({ type: Types.JOINED_GAME, payload: { roomId: 1, gameId: 5 } });
  });
});
