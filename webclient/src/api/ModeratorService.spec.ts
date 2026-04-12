jest.mock('websocket', () => ({
  ModeratorCommands: {
    banFromServer: jest.fn(),
    getBanHistory: jest.fn(),
    getWarnHistory: jest.fn(),
    getWarnList: jest.fn(),
    viewLogHistory: jest.fn(),
    warnUser: jest.fn(),
  },
}));

import { ModeratorService } from './ModeratorService';
import { ModeratorCommands } from 'websocket';
import { LogFilters } from 'types';

beforeEach(() => jest.clearAllMocks());

describe('ModeratorService', () => {
  describe('banFromServer', () => {
    it('delegates to ModeratorCommands.banFromServer with all arguments', () => {
      ModeratorService.banFromServer(30, 'alice', '1.2.3.4', 'reason', 'visible reason', 'cid', 1);
      expect(ModeratorCommands.banFromServer).toHaveBeenCalledWith(
        30, 'alice', '1.2.3.4', 'reason', 'visible reason', 'cid', 1
      );
    });

    it('delegates with only required argument', () => {
      ModeratorService.banFromServer(60);
      expect(ModeratorCommands.banFromServer).toHaveBeenCalledWith(
        60, undefined, undefined, undefined, undefined, undefined, undefined
      );
    });
  });

  describe('getBanHistory', () => {
    it('delegates to ModeratorCommands.getBanHistory', () => {
      ModeratorService.getBanHistory('alice');
      expect(ModeratorCommands.getBanHistory).toHaveBeenCalledWith('alice');
    });
  });

  describe('getWarnHistory', () => {
    it('delegates to ModeratorCommands.getWarnHistory', () => {
      ModeratorService.getWarnHistory('alice');
      expect(ModeratorCommands.getWarnHistory).toHaveBeenCalledWith('alice');
    });
  });

  describe('getWarnList', () => {
    it('delegates to ModeratorCommands.getWarnList', () => {
      ModeratorService.getWarnList('mod1', 'alice', 'cid123');
      expect(ModeratorCommands.getWarnList).toHaveBeenCalledWith('mod1', 'alice', 'cid123');
    });
  });

  describe('viewLogHistory', () => {
    it('delegates to ModeratorCommands.viewLogHistory', () => {
      const filters: LogFilters = { dateRange: 7, userName: 'alice' };
      ModeratorService.viewLogHistory(filters);
      expect(ModeratorCommands.viewLogHistory).toHaveBeenCalledWith(filters);
    });
  });

  describe('warnUser', () => {
    it('delegates to ModeratorCommands.warnUser with all arguments', () => {
      ModeratorService.warnUser('alice', 'spamming', 'cid', 5);
      expect(ModeratorCommands.warnUser).toHaveBeenCalledWith('alice', 'spamming', 'cid', 5);
    });

    it('delegates with only required arguments', () => {
      ModeratorService.warnUser('alice', 'spamming');
      expect(ModeratorCommands.warnUser).toHaveBeenCalledWith('alice', 'spamming', undefined, undefined);
    });
  });
});
