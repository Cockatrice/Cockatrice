import type { GenExtension } from '@bufbuild/protobuf/codegenv2';
import type { Response } from '@app/generated';
import { Response_ResponseCode, ResponseSchema } from '@app/generated';
vi.mock('@bufbuild/protobuf', async () => {
  const actual = await vi.importActual<typeof import('@bufbuild/protobuf')>('@bufbuild/protobuf');
  return { ...actual, getExtension: vi.fn() };
});

import { create, getExtension } from '@bufbuild/protobuf';

import { handleResponse } from './command-options';

// NOTE: do NOT call `vi.resetAllMocks()` here — under `isolate: false` it
// resets `vi.fn()` implementations set inside other files' `vi.mock(...)`
// factories, which breaks any spec that relied on those factory defaults
// (e.g. ProtobufService.spec.ts expects `hasExtension` to return `false`).
// The root `setupTests.ts` afterEach already calls `vi.clearAllMocks()`.

describe('handleResponse', () => {
  it('calls onResponse and returns early when provided', () => {
    const onResponse = vi.fn();
    const onSuccess = vi.fn();
    handleResponse('test', create(ResponseSchema, { responseCode: 99 }), { onResponse, onSuccess });
    expect(onResponse).toHaveBeenCalled();
    expect(onSuccess).not.toHaveBeenCalled();
  });

  it('calls onSuccess when responseCode is RespOk and no responseExt', () => {
    const onSuccess = vi.fn();
    const raw = create(ResponseSchema, { responseCode: Response_ResponseCode.RespOk });
    handleResponse('test', raw, { onSuccess });
    expect(onSuccess).toHaveBeenCalledWith();
  });

  it('calls onSuccess with nested response when responseExt is set', () => {
    vi.mocked(getExtension).mockReturnValue({ nested: true });
    const onSuccess = vi.fn();
    const fakeExt = {} as unknown as GenExtension<Response, unknown>;
    const raw = create(ResponseSchema, { responseCode: Response_ResponseCode.RespOk });
    handleResponse('test', raw, { onSuccess, responseExt: fakeExt });
    expect(onSuccess).toHaveBeenCalledWith({ nested: true }, raw);
  });

  it('calls onResponseCode handler when code matches', () => {
    const specificHandler = vi.fn();
    handleResponse('test', create(ResponseSchema, { responseCode: 5 }), { onResponseCode: { 5: specificHandler } });
    expect(specificHandler).toHaveBeenCalled();
  });

  it('calls onError when responseCode is not RespOk and no specific handler', () => {
    const onError = vi.fn();
    const raw = create(ResponseSchema, { responseCode: 99 });
    handleResponse('test', raw, { onError });
    expect(onError).toHaveBeenCalledWith(99, raw);
  });

  it('logs error to console when no callbacks for non-RespOk response', () => {
    const consoleSpy = vi.spyOn(console, 'error').mockImplementation(() => {});
    handleResponse('test.Type', create(ResponseSchema, { responseCode: 42 }), {});
    expect(consoleSpy).toHaveBeenCalled();
    consoleSpy.mockRestore();
  });
});
