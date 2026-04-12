import { Actions } from './rooms.actions';
import { Types } from './rooms.types';
import { makeGame, makeMessage, makeRoom, makeUser } from './__mocks__/rooms-fixtures';
import { GameSortField, SortDirection } from 'types';

describe('Actions', () => {
  it('clearStore', () => {
    expect(Actions.clearStore()).toEqual({ type: Types.CLEAR_STORE });
  });

  it('updateRooms', () => {
    const rooms = [makeRoom()];
    expect(Actions.updateRooms(rooms)).toEqual({ type: Types.UPDATE_ROOMS, rooms });
  });

  it('joinRoom', () => {
    const roomInfo = makeRoom({ roomId: 2 });
    expect(Actions.joinRoom(roomInfo)).toEqual({ type: Types.JOIN_ROOM, roomInfo });
  });

  it('leaveRoom', () => {
    expect(Actions.leaveRoom(3)).toEqual({ type: Types.LEAVE_ROOM, roomId: 3 });
  });

  it('addMessage', () => {
    const message = makeMessage();
    expect(Actions.addMessage(1, message)).toEqual({ type: Types.ADD_MESSAGE, roomId: 1, message });
  });

  it('updateGames', () => {
    const games = [makeGame()];
    expect(Actions.updateGames(1, games)).toEqual({ type: Types.UPDATE_GAMES, roomId: 1, games });
  });

  it('userJoined', () => {
    const user = makeUser();
    expect(Actions.userJoined(1, user)).toEqual({ type: Types.USER_JOINED, roomId: 1, user });
  });

  it('userLeft', () => {
    expect(Actions.userLeft(1, 'Alice')).toEqual({ type: Types.USER_LEFT, roomId: 1, name: 'Alice' });
  });

  it('sortGames', () => {
    expect(Actions.sortGames(1, GameSortField.START_TIME, SortDirection.ASC)).toEqual({
      type: Types.SORT_GAMES,
      roomId: 1,
      field: GameSortField.START_TIME,
      order: SortDirection.ASC,
    });
  });

  it('removeMessages', () => {
    expect(Actions.removeMessages(1, 'Alice', 3)).toEqual({
      type: Types.REMOVE_MESSAGES,
      roomId: 1,
      name: 'Alice',
      amount: 3,
    });
  });

  it('gameCreated', () => {
    expect(Actions.gameCreated(2)).toEqual({ type: Types.GAME_CREATED, roomId: 2 });
  });

  it('joinedGame', () => {
    expect(Actions.joinedGame(1, 5)).toEqual({ type: Types.JOINED_GAME, roomId: 1, gameId: 5 });
  });
});
