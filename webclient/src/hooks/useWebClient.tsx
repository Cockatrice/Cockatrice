import { createContext, useContext, useState, ReactNode } from 'react';
import { WebClient } from '@app/websocket';
import { createWebClientRequest, createWebClientResponse } from '@app/api';

// Exported so integration tests can inject the WebClient singleton built
// by their shared setup without going through the production provider
// (which would attempt to `new WebClient(...)` a second time and throw).
export const WebClientContext = createContext<WebClient | null>(null);

export function WebClientProvider({ children }: { children: ReactNode }) {
  const [client] = useState(() => new WebClient(createWebClientRequest(), createWebClientResponse()));

  return <WebClientContext value={client}>{children}</WebClientContext>;
}

export function useWebClient(): WebClient {
  const client = useContext(WebClientContext);
  if (!client) {
    throw new Error('useWebClient must be used within a WebClientProvider');
  }
  return client;
}
