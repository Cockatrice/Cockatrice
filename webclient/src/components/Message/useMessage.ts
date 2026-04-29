import { useMemo, type ReactNode } from 'react';

import { App } from '@app/types';

export interface ParsedMessage {
  name: string | null;
  chunks: ReactNode[];
}

export type ChunkParser = (chunk: string, index: number) => ReactNode;

// `parseChunk` must be a stable reference across renders (module-level function
// or `useCallback`). Passing a fresh closure every render will thrash the memo.
export function useParsedMessage(message: string, parseChunk: ChunkParser): ParsedMessage {
  return useMemo<ParsedMessage>(() => {
    const match = message.match(App.MESSAGE_SENDER_REGEX);
    const name = match ? match[1] : null;
    return {
      name,
      chunks: parseMessage(message, parseChunk),
    };
  }, [message, parseChunk]);
}

export function parseMessage(message: string, parseChunk: ChunkParser): ReactNode[] {
  return message
    .replace(App.MESSAGE_SENDER_REGEX, '')
    .split(App.CARD_CALLOUT_REGEX)
    .filter((chunk) => !!chunk)
    .map(parseChunk);
}
