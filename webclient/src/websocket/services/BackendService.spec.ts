import { makeMockProtoRoot } from '../__mocks__/helpers';

jest.mock('./ProtoController', () => ({
  ProtoController: { root: null },
}));

jest.mock('../WebClient', () => {
  const mockProtobuf = {
    sendGameCommand: jest.fn(),
    sendSessionCommand: jest.fn(),
    sendRoomCommand: jest.fn(),
    sendModeratorCommand: jest.fn(),
    sendAdminCommand: jest.fn(),
  };
  return { __esModule: true, default: { protobuf: mockProtobuf } };
});

import { BackendService } from './BackendService';
import { ProtoController } from './ProtoController';
import webClient from '../WebClient';

beforeEach(() => {
  jest.clearAllMocks();
  ProtoController.root = makeMockProtoRoot();
  ProtoController.root.GameCommand = { create: jest.fn(args => ({ ...args })) };
  ProtoController.root['Command_Game'] = { create: jest.fn(p => ({ ...p })) };
  ProtoController.root['Command_Test'] = { create: jest.fn(p => ({ ...p })) };
  ProtoController.root['Command_Room'] = { create: jest.fn(p => ({ ...p })) };
  ProtoController.root['Command_Mod'] = { create: jest.fn(p => ({ ...p })) };
  ProtoController.root['Command_Admin'] = { create: jest.fn(p => ({ ...p })) };
  ProtoController.root['Response_Test'] = {};
});

function captureCallback(sendFn: jest.Mock) {
  return sendFn.mock.calls[0][sendFn === (webClient.protobuf as any).sendRoomCommand ? 2 : 1];
}

describe('BackendService', () => {
  describe('send commands', () => {
    it.each([
      ['sendGameCommand', () => BackendService.sendGameCommand(7, 'Command_Game', { g: 1 })],
      ['sendSessionCommand', () => BackendService.sendSessionCommand('Command_Test', { x: 1 }, {})],
      ['sendRoomCommand', () => BackendService.sendRoomCommand(5, 'Command_Room', { y: 2 }, {})],
      ['sendModeratorCommand', () => BackendService.sendModeratorCommand('Command_Mod', { z: 3 }, {})],
      ['sendAdminCommand', () => BackendService.sendAdminCommand('Command_Admin', {}, {})],
    ])('%s creates the command and delegates to protobuf', (methodName, invoke) => {
      invoke();
      expect((webClient.protobuf as any)[methodName]).toHaveBeenCalled();
    });
  });

  describe('handleResponse via non-session command callbacks', () => {
    it('sendGameCommand callback invokes handleResponse', () => {
      const onSuccess = jest.fn();
      BackendService.sendGameCommand(7, 'Command_Game', {}, { onSuccess });
      const cb = (webClient.protobuf as any).sendGameCommand.mock.calls[0][2];
      cb({ responseCode: 0 });
      expect(onSuccess).toHaveBeenCalled();
    });

    it('sendRoomCommand callback invokes handleResponse', () => {
      const onSuccess = jest.fn();
      BackendService.sendRoomCommand(5, 'Command_Room', {}, { onSuccess });
      captureCallback((webClient.protobuf as any).sendRoomCommand)({ responseCode: 0 });
      expect(onSuccess).toHaveBeenCalled();
    });

    it('sendModeratorCommand callback invokes handleResponse', () => {
      const onSuccess = jest.fn();
      BackendService.sendModeratorCommand('Command_Mod', {}, { onSuccess });
      captureCallback((webClient.protobuf as any).sendModeratorCommand)({ responseCode: 0 });
      expect(onSuccess).toHaveBeenCalled();
    });

    it('sendAdminCommand callback invokes handleResponse', () => {
      const onSuccess = jest.fn();
      BackendService.sendAdminCommand('Command_Admin', {}, { onSuccess });
      captureCallback((webClient.protobuf as any).sendAdminCommand)({ responseCode: 0 });
      expect(onSuccess).toHaveBeenCalled();
    });
  });

  describe('handleResponse (via sendSessionCommand callback)', () => {
    function invokeCallback(options: any, raw: any) {
      BackendService.sendSessionCommand('Command_Test', {}, options);
      const cb = (webClient.protobuf as any).sendSessionCommand.mock.calls[0][1];
      cb(raw);
    }

    it('calls onResponse and returns early when provided', () => {
      const onResponse = jest.fn();
      const onSuccess = jest.fn();
      invokeCallback({ onResponse, onSuccess }, { responseCode: 99 });
      expect(onResponse).toHaveBeenCalled();
      expect(onSuccess).not.toHaveBeenCalled();
    });

    it('calls onSuccess with raw when responseCode is RespOk and no responseName', () => {
      const onSuccess = jest.fn();
      const raw = { responseCode: 0 };
      invokeCallback({ onSuccess }, raw);
      expect(onSuccess).toHaveBeenCalledWith(raw, raw);
    });

    it('calls onSuccess with nested response when responseName is set', () => {
      const onSuccess = jest.fn();
      const raw = { responseCode: 0, '.Response_Test.ext': { nested: true } };
      invokeCallback({ onSuccess, responseName: 'Response_Test' }, raw);
      expect(onSuccess).toHaveBeenCalledWith({ nested: true }, raw);
    });

    it('calls onResponseCode handler when code matches', () => {
      const specificHandler = jest.fn();
      invokeCallback({ onResponseCode: { 5: specificHandler } }, { responseCode: 5 });
      expect(specificHandler).toHaveBeenCalled();
    });

    it('calls onError when responseCode is not RespOk and no specific handler', () => {
      const onError = jest.fn();
      invokeCallback({ onError }, { responseCode: 99 });
      expect(onError).toHaveBeenCalledWith(99, { responseCode: 99 });
    });

    it('logs error to console when no callbacks for non-RespOk response', () => {
      const consoleSpy = jest.spyOn(console, 'error').mockImplementation(() => {});
      invokeCallback({}, { responseCode: 42 });
      expect(consoleSpy).toHaveBeenCalled();
      consoleSpy.mockRestore();
    });
  });
});
