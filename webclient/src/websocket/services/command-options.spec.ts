vi.mock('@bufbuild/protobuf', () => ({
  getExtension: vi.fn(),
}));

vi.mock('generated/proto/response_pb', () => ({
  Response_ResponseCode: { RespOk: 1 },
}));

import { getExtension } from '@bufbuild/protobuf';
import { handleResponse } from './command-options';

beforeEach(() => {
  vi.resetAllMocks();
});

describe('handleResponse', () => {
  it('calls onResponse and returns early when provided', () => {
    const onResponse = vi.fn();
    const onSuccess = vi.fn();
    handleResponse('test', { responseCode: 99 } as any, { onResponse, onSuccess });
    expect(onResponse).toHaveBeenCalled();
    expect(onSuccess).not.toHaveBeenCalled();
  });

  it('calls onSuccess when responseCode is RespOk and no responseExt', () => {
    const onSuccess = vi.fn();
    const raw = { responseCode: 1 } as any;
    handleResponse('test', raw, { onSuccess });
    expect(onSuccess).toHaveBeenCalledWith();
  });

  it('calls onSuccess with nested response when responseExt is set', () => {
    vi.mocked(getExtension).mockReturnValue({ nested: true } as any);
    const onSuccess = vi.fn();
    const fakeExt = {} as any;
    const raw = { responseCode: 1 } as any;
    handleResponse('test', raw, { onSuccess, responseExt: fakeExt });
    expect(onSuccess).toHaveBeenCalledWith({ nested: true }, raw);
  });

  it('calls onResponseCode handler when code matches', () => {
    const specificHandler = vi.fn();
    handleResponse('test', { responseCode: 5 } as any, { onResponseCode: { 5: specificHandler } });
    expect(specificHandler).toHaveBeenCalled();
  });

  it('calls onError when responseCode is not RespOk and no specific handler', () => {
    const onError = vi.fn();
    handleResponse('test', { responseCode: 99 } as any, { onError });
    expect(onError).toHaveBeenCalledWith(99, { responseCode: 99 });
  });

  it('logs error to console when no callbacks for non-RespOk response', () => {
    const consoleSpy = vi.spyOn(console, 'error').mockImplementation(() => {});
    handleResponse('test.Type', { responseCode: 42 } as any, {});
    expect(consoleSpy).toHaveBeenCalled();
    consoleSpy.mockRestore();
  });
});
