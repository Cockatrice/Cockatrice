vi.mock('websocket', () => ({
  SessionCommands: {
    addToBuddyList: vi.fn(),
    removeFromBuddyList: vi.fn(),
    addToIgnoreList: vi.fn(),
    removeFromIgnoreList: vi.fn(),
    accountPassword: vi.fn(),
    accountEdit: vi.fn(),
    accountImage: vi.fn(),
    message: vi.fn(),
    getUserInfo: vi.fn(),
    getGamesOfUser: vi.fn(),
  },
}));

import { SessionService } from './SessionService';
import { SessionCommands } from 'websocket';

beforeEach(() => vi.clearAllMocks());

describe('SessionService', () => {
  describe('addToBuddyList', () => {
    it('delegates to SessionCommands.addToBuddyList', () => {
      SessionService.addToBuddyList('alice');
      expect(SessionCommands.addToBuddyList).toHaveBeenCalledWith('alice');
    });
  });

  describe('removeFromBuddyList', () => {
    it('delegates to SessionCommands.removeFromBuddyList', () => {
      SessionService.removeFromBuddyList('alice');
      expect(SessionCommands.removeFromBuddyList).toHaveBeenCalledWith('alice');
    });
  });

  describe('addToIgnoreList', () => {
    it('delegates to SessionCommands.addToIgnoreList', () => {
      SessionService.addToIgnoreList('bob');
      expect(SessionCommands.addToIgnoreList).toHaveBeenCalledWith('bob');
    });
  });

  describe('removeFromIgnoreList', () => {
    it('delegates to SessionCommands.removeFromIgnoreList', () => {
      SessionService.removeFromIgnoreList('bob');
      expect(SessionCommands.removeFromIgnoreList).toHaveBeenCalledWith('bob');
    });
  });

  describe('changeAccountPassword', () => {
    it('delegates to SessionCommands.accountPassword with all arguments', () => {
      SessionService.changeAccountPassword('oldPw', 'newPw', 'hashedPw');
      expect(SessionCommands.accountPassword).toHaveBeenCalledWith('oldPw', 'newPw', 'hashedPw');
    });

    it('delegates without hashedNewPassword when omitted', () => {
      SessionService.changeAccountPassword('oldPw', 'newPw');
      expect(SessionCommands.accountPassword).toHaveBeenCalledWith('oldPw', 'newPw', undefined);
    });
  });

  describe('changeAccountDetails', () => {
    it('delegates to SessionCommands.accountEdit with all arguments', () => {
      SessionService.changeAccountDetails('pw', 'Alice', 'alice@example.com', 'US');
      expect(SessionCommands.accountEdit).toHaveBeenCalledWith('pw', 'Alice', 'alice@example.com', 'US');
    });

    it('delegates with only required argument', () => {
      SessionService.changeAccountDetails('pw');
      expect(SessionCommands.accountEdit).toHaveBeenCalledWith('pw', undefined, undefined, undefined);
    });
  });

  describe('changeAccountImage', () => {
    it('delegates to SessionCommands.accountImage', () => {
      const image = new Uint8Array([1, 2, 3]);
      SessionService.changeAccountImage(image);
      expect(SessionCommands.accountImage).toHaveBeenCalledWith(image);
    });
  });

  describe('sendDirectMessage', () => {
    it('delegates to SessionCommands.message', () => {
      SessionService.sendDirectMessage('alice', 'hello');
      expect(SessionCommands.message).toHaveBeenCalledWith('alice', 'hello');
    });
  });

  describe('getUserInfo', () => {
    it('delegates to SessionCommands.getUserInfo', () => {
      SessionService.getUserInfo('alice');
      expect(SessionCommands.getUserInfo).toHaveBeenCalledWith('alice');
    });
  });

  describe('getUserGames', () => {
    it('delegates to SessionCommands.getGamesOfUser', () => {
      SessionService.getUserGames('alice');
      expect(SessionCommands.getGamesOfUser).toHaveBeenCalledWith('alice');
    });
  });
});
