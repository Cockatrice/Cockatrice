import type { WebsocketTypes } from '@app/websocket/types';

export const PROTOCOL_VERSION = 14;

export const CLIENT_CONFIG: WebsocketTypes.ClientConfig = {
  clientid: 'webatrice',
  clientver: 'webclient-1.0 (2019-10-31)',
  clientfeatures: [
    'client_id',
    'client_ver',
    'feature_set',
    'room_chat_history',
    'client_warnings',
    'forgot_password',
    'idle_client',
    'mod_log_lookup',
    'user_ban_history',
    'websocket',
    '2.7.0_min_version',
    '2.8.0_min_version',
  ],
};

export const CLIENT_OPTIONS: WebsocketTypes.ClientOptions = {
  autojoinrooms: true,
  keepalive: 5000,
};
