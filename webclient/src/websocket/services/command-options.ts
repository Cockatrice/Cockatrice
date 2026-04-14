import { getExtension } from '@bufbuild/protobuf';
import type { GenExtension } from '@bufbuild/protobuf/codegenv2';
import { Response_ResponseCode, type Response } from 'generated/proto/response_pb';

interface CommandOptionsBase {
  onError?: (responseCode: number, raw: Response) => void;
  onResponseCode?: { [code: number]: (raw: Response) => void };
  onResponse?: (raw: Response) => void;
}

export interface CommandOptionsWithResponse<R> extends CommandOptionsBase {
  responseExt: GenExtension<Response, R>;
  onSuccess?: (response: R, raw: Response) => void;
}

export interface CommandOptionsWithoutResponse extends CommandOptionsBase {
  responseExt?: undefined;
  onSuccess?: () => void;
}

export type CommandOptions<R = unknown> = CommandOptionsWithResponse<R> | CommandOptionsWithoutResponse;

export function hasResponseExt<R>(options: CommandOptions<R>): options is CommandOptionsWithResponse<R> {
  return options.responseExt !== undefined;
}

export function handleResponse<R>(typeName: string, raw: Response, options: CommandOptions<R>): void {
  if (options.onResponse) {
    options.onResponse(raw);
    return;
  }

  const { responseCode } = raw;

  if (responseCode === Response_ResponseCode.RespOk) {
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
