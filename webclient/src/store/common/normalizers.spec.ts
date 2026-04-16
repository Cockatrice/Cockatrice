import { normalizeRoomInfo, normalizeGameObject, normalizeLogs, normalizeBannedUserError, normalizeUserMessage } from './normalizers';
import { create } from '@bufbuild/protobuf';
import { Data, Enriched } from '@app/types';

describe('normalizeRoomInfo', () => {
  it('builds gametypeMap from gametypeList and keys games by gameId', () => {
    const room = create(Data.ServerInfo_RoomSchema, {
      roomId: 1,
      name: 'Lobby',
      gametypeList: [{ gameTypeId: 1, description: 'Standard' }],
      gameList: [
        create(Data.ServerInfo_GameSchema, { gameId: 10, gameTypes: [1], description: 'My Game' }),
      ],
    });

    const result = normalizeRoomInfo(room);

    expect(result.info).toBe(room);
    expect(result.gametypeMap).toEqual({ 1: 'Standard' });
    expect(Object.keys(result.games)).toHaveLength(1);
    expect(result.games[10].gameType).toBe('Standard');
    expect(result.games[10].info.gameId).toBe(10);
    expect(result.order).toBe(0);
  });

  it('handles room with empty gametypeList', () => {
    const room = create(Data.ServerInfo_RoomSchema, { roomId: 2, name: 'Empty' });
    const result = normalizeRoomInfo(room);
    expect(result.gametypeMap).toEqual({});
    expect(result.games).toEqual({});
    expect(result.users).toEqual({});
  });

  it('keys users by name', () => {
    const room = create(Data.ServerInfo_RoomSchema, {
      roomId: 1,
      userList: [
        create(Data.ServerInfo_UserSchema, { name: 'alice' }),
        create(Data.ServerInfo_UserSchema, { name: 'bob' }),
      ],
    });
    const result = normalizeRoomInfo(room);
    expect(result.users['alice']).toBeDefined();
    expect(result.users['bob']).toBeDefined();
  });
});

describe('normalizeGameObject', () => {
  it('maps gameTypes[0] to gameType string via gametypeMap', () => {
    const game = create(Data.ServerInfo_GameSchema, { gameId: 1, gameTypes: [5] });
    const result = normalizeGameObject(game, { 5: 'Legacy' });
    expect(result.gameType).toBe('Legacy');
    expect(result.info).toBe(game);
  });

  it('returns empty string when no gameTypes', () => {
    const game = create(Data.ServerInfo_GameSchema, { gameId: 2 });
    const result = normalizeGameObject(game, {});
    expect(result.gameType).toBe('');
  });

  it('stores raw proto on info', () => {
    const game = create(Data.ServerInfo_GameSchema, { gameId: 3 });
    const result = normalizeGameObject(game, {});
    expect(result.info.gameId).toBe(3);
    expect(result.info.description).toBe('');
  });
});

describe('normalizeLogs', () => {
  it('groups logs by targetType', () => {
    const logs = [
      create(Data.ServerInfo_ChatMessageSchema, { targetType: 'room' }),
      create(Data.ServerInfo_ChatMessageSchema, { targetType: 'game' }),
      create(Data.ServerInfo_ChatMessageSchema, { targetType: 'room' }),
    ];
    const result = normalizeLogs(logs);
    expect(result.room).toHaveLength(2);
    expect(result.game).toHaveLength(1);
    expect(result.chat).toBeUndefined();
  });

  it('returns empty object for empty logs', () => {
    expect(normalizeLogs([])).toEqual({});
  });
});

describe('normalizeBannedUserError', () => {
  it('returns permanently banned Enriched.Message when endTime is 0', () => {
    expect(normalizeBannedUserError('', 0)).toBe('You are permanently banned');
  });

  it('returns banned until date when endTime is given', () => {
    const endTime = new Date('2030-01-01').getTime();
    const result = normalizeBannedUserError('', endTime);
    expect(result).toContain('You are banned until');
    expect(result).toContain(new Date(endTime).toString());
  });

  it('appends reason when provided', () => {
    expect(normalizeBannedUserError('bad behavior', 0)).toContain('\n\nbad behavior');
  });

  it('does not append separator when reason is empty', () => {
    expect(normalizeBannedUserError('', 0)).not.toContain('\n\n');
  });
});

describe('normalizeUserMessage', () => {
  const makeMsg = (fields: Partial<Enriched.Message>): Enriched.Message => ({
    ...create(Data.Event_RoomSaySchema),
    timeReceived: 0,
    ...fields,
  } as Enriched.Message);

  it('prepends "name: " to message when name is present', () => {
    const result = normalizeUserMessage(makeMsg({ name: 'Alice', message: 'hello' }));
    expect(result.message).toBe('Alice: hello');
  });

  it('returns message unchanged when name is empty', () => {
    const result = normalizeUserMessage(makeMsg({ name: '', message: 'system msg' }));
    expect(result.message).toBe('system msg');
  });

  it('does not mutate the original message', () => {
    const original = makeMsg({ name: 'Bob', message: 'hi' });
    normalizeUserMessage(original);
    expect(original.message).toBe('hi');
  });

  it('returns the original reference when no name (no allocation)', () => {
    const original = makeMsg({ name: '', message: 'hi' });
    expect(normalizeUserMessage(original)).toBe(original);
  });
});
