import { getExtension } from '@bufbuild/protobuf';
import type { GenExtension } from '@bufbuild/protobuf/codegenv2';
import { Data } from '@app/types';

interface CommandOptionsBase {
  onError?: (responseCode: number, raw: Data.Response) => void;
  onResponseCode?: { [code: number]: (raw: Data.Response) => void };
  onResponse?: (raw: Data.Response) => void;
}

export interface CommandOptionsWithResponse<R> extends CommandOptionsBase {
  responseExt: GenExtension<Data.Response, R>;
  onSuccess?: (response: R, raw: Data.Response) => void;
}

export interface CommandOptionsWithoutResponse extends CommandOptionsBase {
  responseExt?: undefined;
  onSuccess?: () => void;
}

export type CommandOptions<R = unknown> = CommandOptionsWithResponse<R> | CommandOptionsWithoutResponse;

export function hasResponseExt<R>(options: CommandOptions<R>): options is CommandOptionsWithResponse<R> {
  return options.responseExt !== undefined;
}

export function handleResponse<R>(typeName: string, raw: Data.Response, options: CommandOptions<R>): void {
  if (options.onResponse) {
    options.onResponse(raw);
    return;
  }

  const { responseCode } = raw;

  if (responseCode === Data.Response_ResponseCode.RespOk) {
    if (hasResponseExt(options)) {
      options.onSuccess?.(getExtension(raw, options.responseExt), raw);
    } else {
      options.onSuccess?.();
    }
    return;
  }

  if (options.onResponseCode?.[responseCode]) {
    options.onResponseCode[responseCode](raw);
    return;
  }

  if (options.onError) {
    options.onError(responseCode, raw);
  } else {
    console.error(`${typeName} failed with response code: ${responseCode}`);
  }
}
