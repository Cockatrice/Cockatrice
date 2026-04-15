vi.mock('@app/websocket', () => ({
  AdminCommands: {
    adjustMod: vi.fn(),
    reloadConfig: vi.fn(),
    shutdownServer: vi.fn(),
    updateServerMessage: vi.fn(),
  },
}));

import { AdminService } from './AdminService';
import { AdminCommands } from '@app/websocket';

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
