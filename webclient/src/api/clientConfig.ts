import { SOCKATRICE_FEATURES } from 'sockatrice';

import type { WebsocketTypes } from '@app/websocket/types';

const APP_FEATURES = [
  '2.7.0_min_version',
  '2.8.0_min_version',
] as const;

export const CLIENT_CONFIG: WebsocketTypes.ClientConfig = {
  clientid: 'webatrice',
  clientver: 'webclient-1.0 (2019-10-31)',
  clientfeatures: [...SOCKATRICE_FEATURES, ...APP_FEATURES],
};

export const CLIENT_OPTIONS: WebsocketTypes.ClientOptions = {
  autojoinrooms: true,
  keepalive: 5000,
};
