import { useEffect, useState, type ReactNode } from 'react';

import { App } from '@app/types';

export interface ParsedMessage {
  name: string | null;
  chunks: ReactNode[] | null;
}

export type ChunkParser = (chunk: string, index: number) => ReactNode;

export function useParsedMessage(message: string, parseChunk: ChunkParser): ParsedMessage {
  const [chunks, setChunks] = useState<ReactNode[] | null>(null);
  const [name, setName] = useState<string | null>(null);

  useEffect(() => {
    const match = message.match(App.MESSAGE_SENDER_REGEX);
    if (match) {
      setName(match[1]);
    }
    setChunks(parseMessage(message, parseChunk));
  }, [message, parseChunk]);

  return { name, chunks };
}

export function parseMessage(message: string, parseChunk: ChunkParser): ReactNode[] {
  return message
    .replace(App.MESSAGE_SENDER_REGEX, '')
    .split(App.CARD_CALLOUT_REGEX)
    .filter((chunk) => !!chunk)
    .map(parseChunk);
}
