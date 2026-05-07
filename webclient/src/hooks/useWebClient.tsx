import { createContext, useContext, useState, ReactNode } from 'react';
import { WebClient } from '@app/websocket';
import {
  CLIENT_CONFIG,
  CLIENT_OPTIONS,
  PROTOCOL_VERSION,
  createWebClientResponse,
} from '@app/api';

export const WebClientContext = createContext<WebClient | null>(null);
WebClientContext.displayName = 'WebClientContext';

export function WebClientProvider({ children }: { children: ReactNode }) {
  const [client] = useState(() => new WebClient(
    createWebClientResponse(),
    CLIENT_CONFIG,
    CLIENT_OPTIONS,
    PROTOCOL_VERSION,
  ));

  return <WebClientContext value={client}>{children}</WebClientContext>;
}

export function useWebClient(): WebClient {
  const client = useContext(WebClientContext);
  if (!client) {
    throw new Error('useWebClient must be used within a WebClientProvider');
  }
  return client;
}
