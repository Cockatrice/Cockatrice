/**
 * Factory for invoking BackendService command callbacks in unit tests.
 *
 * @param mockFn       - The jest.Mock for the BackendService send method
 *                       (e.g. BackendService.sendSessionCommand as jest.Mock).
 * @param optsArgIndex - Index of the options argument in the mock call.
 *                       Defaults to 2 (commandName, params, options).
 *                       Use 3 for sendRoomCommand (roomId, commandName, params, options).
 */
export function makeCallbackHelpers(mockFn: jest.Mock, optsArgIndex = 2) {
  function getLastSendOpts() {
    const calls = mockFn.mock.calls;
    return calls[calls.length - 1]?.[optsArgIndex];
  }

  function invokeOnSuccess(response: any = {}, raw?: any) {
    getLastSendOpts()?.onSuccess?.(response, raw ?? response);
  }

  function invokeResponseCode(code: number, raw: any = { responseCode: code }) {
    const opts = getLastSendOpts();
    if (opts?.onResponseCode?.[code]) {
      opts.onResponseCode[code](raw);
    }
  }

  function invokeOnError(code: number = 99, raw: any = {}) {
    getLastSendOpts()?.onError?.(code, raw);
  }

  function invokeCallback(callbackName: string, ...args: any[]) {
    const opts = getLastSendOpts();
    if (opts?.[callbackName]) {
      opts[callbackName](...args);
    }
  }

  return { getLastSendOpts, invokeOnSuccess, invokeResponseCode, invokeOnError, invokeCallback };
}
