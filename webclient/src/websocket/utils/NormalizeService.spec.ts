import NormalizeService from './NormalizeService';

describe('NormalizeService', () => {
  describe('normalizeRoomInfo', () => {
    it('builds gametypeMap from gametypeList', () => {
      const roomInfo: any = {
        gametypeList: [
          { gameTypeId: 1, description: 'Standard' },
          { gameTypeId: 2, description: 'Draft' },
        ],
        gametypeMap: {},
        gameList: [],
      };
      NormalizeService.normalizeRoomInfo(roomInfo);
      expect(roomInfo.gametypeMap).toEqual({ 1: 'Standard', 2: 'Draft' });
    });

    it('normalizes each game in gameList', () => {
      const roomInfo: any = {
        gametypeList: [{ gameTypeId: 5, description: 'Modern' }],
        gametypeMap: {},
        gameList: [{ gameTypes: [5], description: 'My Game' }],
      };
      NormalizeService.normalizeRoomInfo(roomInfo);
      expect(roomInfo.gameList[0].gameType).toBe('Modern');
    });
  });

  describe('normalizeGameObject', () => {
    it('sets gameType from first element of gameTypes', () => {
      const game: any = { gameTypes: [3], description: 'Test' };
      const map: any = { 3: 'Legacy' };
      NormalizeService.normalizeGameObject(game, map);
      expect(game.gameType).toBe('Legacy');
    });

    it('sets gameType to empty string when gameTypes is empty', () => {
      const game: any = { gameTypes: [], description: 'Test' };
      NormalizeService.normalizeGameObject(game, {});
      expect(game.gameType).toBe('');
    });

    it('sets gameType to empty string when gameTypes is null', () => {
      const game: any = { gameTypes: null, description: 'Test' };
      NormalizeService.normalizeGameObject(game, {});
      expect(game.gameType).toBe('');
    });

    it('sets description to empty string when description is falsy', () => {
      const game: any = { gameTypes: [], description: null };
      NormalizeService.normalizeGameObject(game, {});
      expect(game.description).toBe('');
    });
  });

  describe('normalizeLogs', () => {
    it('groups logs by targetType', () => {
      const logs: any[] = [
        { targetType: 'room', msg: 'a' },
        { targetType: 'chat', msg: 'b' },
        { targetType: 'room', msg: 'c' },
      ];
      const result = NormalizeService.normalizeLogs(logs);
      expect(result['room']).toHaveLength(2);
      expect(result['chat']).toHaveLength(1);
    });

    it('returns empty object for empty array', () => {
      expect(NormalizeService.normalizeLogs([])).toEqual({});
    });
  });

  describe('normalizeUserMessage', () => {
    it('prepends username when name is present', () => {
      const message: any = { name: 'Alice', message: 'hello' };
      NormalizeService.normalizeUserMessage(message);
      expect(message.message).toBe('Alice: hello');
    });

    it('does not modify message when name is absent', () => {
      const message: any = { name: '', message: 'hello' };
      NormalizeService.normalizeUserMessage(message);
      expect(message.message).toBe('hello');
    });
  });

  describe('normalizeBannedUserError', () => {
    it('returns permanently banned message when endTime is 0', () => {
      const result = NormalizeService.normalizeBannedUserError('', 0);
      expect(result).toBe('You are permanently banned');
    });

    it('returns banned until date when endTime is given', () => {
      const endTime = new Date('2030-01-01').getTime();
      const result = NormalizeService.normalizeBannedUserError('', endTime);
      expect(result).toContain('You are banned until');
      expect(result).toContain(new Date(endTime).toString());
    });

    it('appends reasonStr when provided', () => {
      const result = NormalizeService.normalizeBannedUserError('bad behavior', 0);
      expect(result).toContain('\n\nbad behavior');
    });

    it('does not append when reasonStr is empty', () => {
      const result = NormalizeService.normalizeBannedUserError('', 0);
      expect(result).not.toContain('\n\n');
    });
  });
});
