jest.mock('../persistence', () => ({
  SessionPersistence: { initialized: jest.fn() },
}));

jest.mock('../../proto-files.json', () => ['test.proto'], { virtual: true });

import { ProtoController } from './ProtoController';
import { SessionPersistence } from '../persistence';
import protobuf from 'protobufjs';

beforeEach(() => {
  jest.clearAllMocks();
  ProtoController.root = null;
  (process.env as any).PUBLIC_URL = '';
});

describe('ProtoController', () => {
  describe('load', () => {
    it('creates a new protobuf.Root', () => {
      ProtoController.load();
      expect(ProtoController.root).toBeDefined();
    });

    it('calls initialized when callback succeeds', () => {
      const loadSpy = jest.spyOn(protobuf.Root.prototype, 'load').mockImplementation(
        (_files: any, _opts: any, cb: any) => cb(null)
      );
      ProtoController.load();
      expect(SessionPersistence.initialized).toHaveBeenCalled();
      loadSpy.mockRestore();
    });

    it('throws when callback receives an error', () => {
      const loadSpy = jest.spyOn(protobuf.Root.prototype, 'load').mockImplementation(
        (_files: any, _opts: any, cb: any) => cb(new Error('load failed'))
      );
      expect(() => ProtoController.load()).toThrow('load failed');
      loadSpy.mockRestore();
    });
  });
});
