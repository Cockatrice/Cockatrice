vi.mock('@bufbuild/protobuf', () => ({
  create: vi.fn().mockReturnValue({}),
  setExtension: vi.fn(),
  getExtension: vi.fn(),
}));

vi.mock('generated/proto/response_pb', () => ({
  Response_ResponseCode: { RespOk: 1 },
}));

vi.mock('../WebClient', () => {
  const mockProtobuf = {
    sendGameCommand: vi.fn(),
    sendSessionCommand: vi.fn(),
    sendRoomCommand: vi.fn(),
    sendModeratorCommand: vi.fn(),
    sendAdminCommand: vi.fn(),
  };
  return { __esModule: true, default: { protobuf: mockProtobuf } };
});

import { getExtension } from '@bufbuild/protobuf';
import { BackendService } from './BackendService';
import webClient from '../WebClient';

beforeEach(() => {
  vi.clearAllMocks();
});

function captureCallback(sendFn: ReturnType<typeof vi.fn>) {
  const protobuf = webClient.protobuf as any;
  const usesIndex2 = sendFn === protobuf.sendRoomCommand || sendFn === protobuf.sendGameCommand;
  return sendFn.mock.calls[0][usesIndex2 ? 2 : 1];
}

describe('BackendService', () => {
  describe('send commands', () => {
    it.each([
      ['sendGameCommand', () => BackendService.sendGameCommand(7, {} as any, {} as any)],
      ['sendSessionCommand', () => BackendService.sendSessionCommand({} as any, {} as any)],
      ['sendRoomCommand', () => BackendService.sendRoomCommand(5, {} as any, {} as any)],
      ['sendModeratorCommand', () => BackendService.sendModeratorCommand({} as any, {} as any)],
      ['sendAdminCommand', () => BackendService.sendAdminCommand({} as any, {} as any)],
    ])('%s delegates to protobuf', (methodName, invoke) => {
      invoke();
      expect((webClient.protobuf as any)[methodName]).toHaveBeenCalled();
    });
  });

  describe('handleResponse via non-session command callbacks', () => {
    it('sendGameCommand callback invokes handleResponse', () => {
      const onSuccess = vi.fn();
      BackendService.sendGameCommand(7, {} as any, {} as any, { onSuccess });
      const cb = (webClient.protobuf as any).sendGameCommand.mock.calls[0][2];
      cb({ responseCode: 1 });
      expect(onSuccess).toHaveBeenCalled();
    });

    it('sendRoomCommand callback invokes handleResponse', () => {
      const onSuccess = vi.fn();
      BackendService.sendRoomCommand(5, {} as any, {} as any, { onSuccess });
      captureCallback((webClient.protobuf as any).sendRoomCommand)({ responseCode: 1 });
      expect(onSuccess).toHaveBeenCalled();
    });

    it('sendModeratorCommand callback invokes handleResponse', () => {
      const onSuccess = vi.fn();
      BackendService.sendModeratorCommand({} as any, {} as any, { onSuccess });
      captureCallback((webClient.protobuf as any).sendModeratorCommand)({ responseCode: 1 });
      expect(onSuccess).toHaveBeenCalled();
    });

    it('sendAdminCommand callback invokes handleResponse', () => {
      const onSuccess = vi.fn();
      BackendService.sendAdminCommand({} as any, {} as any, { onSuccess });
      captureCallback((webClient.protobuf as any).sendAdminCommand)({ responseCode: 1 });
      expect(onSuccess).toHaveBeenCalled();
    });
  });

  describe('handleResponse (via sendSessionCommand callback)', () => {
    function invokeCallback(options: any, raw: any) {
      BackendService.sendSessionCommand({} as any, {} as any, options);
      const cb = (webClient.protobuf as any).sendSessionCommand.mock.calls[0][1];
      cb(raw);
    }

    it('calls onResponse and returns early when provided', () => {
      const onResponse = vi.fn();
      const onSuccess = vi.fn();
      invokeCallback({ onResponse, onSuccess }, { responseCode: 99 });
      expect(onResponse).toHaveBeenCalled();
      expect(onSuccess).not.toHaveBeenCalled();
    });

    it('calls onSuccess when responseCode is RespOk and no responseExt', () => {
      const onSuccess = vi.fn();
      const raw = { responseCode: 1 };
      invokeCallback({ onSuccess }, raw);
      expect(onSuccess).toHaveBeenCalledWith();
    });

    it('calls onSuccess with nested response when responseExt is set', () => {
      vi.mocked(getExtension).mockReturnValue({ nested: true });
      const onSuccess = vi.fn();
      const fakeExt = {} as any;
      const raw = { responseCode: 1 };
      invokeCallback({ onSuccess, responseExt: fakeExt }, raw);
      expect(onSuccess).toHaveBeenCalledWith({ nested: true }, raw);
    });

    it('calls onResponseCode handler when code matches', () => {
      const specificHandler = vi.fn();
      invokeCallback({ onResponseCode: { 5: specificHandler } }, { responseCode: 5 });
      expect(specificHandler).toHaveBeenCalled();
    });

    it('calls onError when responseCode is not RespOk and no specific handler', () => {
      const onError = vi.fn();
      invokeCallback({ onError }, { responseCode: 99 });
      expect(onError).toHaveBeenCalledWith(99, { responseCode: 99 });
    });

    it('logs error to console when no callbacks for non-RespOk response', () => {
      const consoleSpy = vi.spyOn(console, 'error').mockImplementation(() => {});
      invokeCallback({}, { responseCode: 42 });
      expect(consoleSpy).toHaveBeenCalled();
      consoleSpy.mockRestore();
    });
  });
});
