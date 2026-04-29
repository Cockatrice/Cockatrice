export const PROTOCOL_VERSION = 14;

export const CLIENT_CONFIG = {
  clientid: 'webatrice',
  clientver: 'webclient-1.0 (2019-10-31)',
  clientfeatures: [
    'client_id',
    'client_ver',
    'feature_set',
    'room_chat_history',
    'client_warnings',
    /* unimplemented features */
    'forgot_password',
    'idle_client',
    'mod_log_lookup',
    'user_ban_history',
    // satisfy server reqs for POC
    'websocket',
    '2.7.0_min_version',
    '2.8.0_min_version'
  ]
} as const;

export const CLIENT_OPTIONS = {
  autojoinrooms: true,
  keepalive: 5000
} as const;
