jest.mock('websocket', () => ({
  AdminCommands: {
    adjustMod: jest.fn(),
    reloadConfig: jest.fn(),
    shutdownServer: jest.fn(),
    updateServerMessage: jest.fn(),
  },
}));

import { AdminService } from './AdminService';
import { AdminCommands } from 'websocket';

beforeEach(() => jest.clearAllMocks());

describe('AdminService', () => {
  describe('adjustMod', () => {
    it('delegates to AdminCommands.adjustMod with all arguments', () => {
      AdminService.adjustMod('alice', true, false);
      expect(AdminCommands.adjustMod).toHaveBeenCalledWith('alice', true, false);
    });

    it('delegates with optional arguments omitted', () => {
      AdminService.adjustMod('alice');
      expect(AdminCommands.adjustMod).toHaveBeenCalledWith('alice', undefined, undefined);
    });
  });

  describe('reloadConfig', () => {
    it('delegates to AdminCommands.reloadConfig', () => {
      AdminService.reloadConfig();
      expect(AdminCommands.reloadConfig).toHaveBeenCalled();
    });
  });

  describe('shutdownServer', () => {
    it('delegates to AdminCommands.shutdownServer', () => {
      AdminService.shutdownServer('maintenance', 10);
      expect(AdminCommands.shutdownServer).toHaveBeenCalledWith('maintenance', 10);
    });
  });

  describe('updateServerMessage', () => {
    it('delegates to AdminCommands.updateServerMessage', () => {
      AdminService.updateServerMessage();
      expect(AdminCommands.updateServerMessage).toHaveBeenCalled();
    });
  });
});
