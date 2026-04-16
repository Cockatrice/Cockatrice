import { createContext, useContext, useState, ReactNode } from 'react';
import { WebClient } from '@app/websocket';
import { createWebClientRequest, createWebClientResponse } from '@app/api';

const WebClientContext = createContext<WebClient | null>(null);

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
