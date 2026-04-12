vi.mock('../persistence', () => ({
  SessionPersistence: { initialized: vi.fn() },
}));

vi.mock('../../proto-files.json', () => ({ default: ['test.proto'] }));

import { ProtoController } from './ProtoController';
import { SessionPersistence } from '../persistence';
import protobuf from 'protobufjs';

beforeEach(() => {
  vi.clearAllMocks();
  ProtoController.root = null;
});

describe('ProtoController', () => {
  describe('load', () => {
    it('creates a new protobuf.Root', () => {
      ProtoController.load();
      expect(ProtoController.root).toBeDefined();
    });

    it('calls initialized when callback succeeds', () => {
      const loadSpy = vi.spyOn(protobuf.Root.prototype, 'load').mockImplementation(
        ((_files: any, _opts: any, cb: any) => cb(null)) as any
      );
      ProtoController.load();
      expect(SessionPersistence.initialized).toHaveBeenCalled();
      loadSpy.mockRestore();
    });

    it('throws when callback receives an error', () => {
      const loadSpy = vi.spyOn(protobuf.Root.prototype, 'load').mockImplementation(
        ((_files: any, _opts: any, cb: any) => cb(new Error('load failed'))) as any
      );
      expect(() => ProtoController.load()).toThrow('load failed');
      loadSpy.mockRestore();
    });
  });
});
