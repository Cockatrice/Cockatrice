export interface ServerStatus {
  status: StatusEnum;
  description: string;
}

export enum StatusEnum {
  DISCONNECTED,
  CONNECTING,
  CONNECTED,
  LOGGING_IN,
  LOGGED_IN,
  DISCONNECTING = 99
}

export enum WebSocketConnectReason {
  LOGIN,
  REGISTER,
  ACTIVATE_ACCOUNT,
  PASSWORD_RESET_REQUEST,
  PASSWORD_RESET_CHALLENGE,
  PASSWORD_RESET,
  TEST_CONNECTION,
}

export class Host {
  id?: number;
  name: string;
  host: string;
  port: string;
  localHost?: string;
  localPort?: string;
  editable: boolean;
  lastSelected?: boolean;
  userName?: string;
  hashedPassword?: string;
  remember?: boolean;
}

export const DefaultHosts: Host[] = [
  {
    name: 'Chickatrice',
    host: 'mtg.chickatrice.net',
    port: '443',
    localPort: '4748',
    editable: false,
  },
  {
    name: 'Rooster',
    host: 'server.cockatrice.us/servatrice',
    port: '4748',
    localHost: 'server.cockatrice.us',
    editable: false,
  },
  {
    name: 'Rooster Beta',
    host: 'beta.cockatrice.us/servatrice',
    port: '4748',
    localHost: 'beta.cockatrice.us',
    editable: false,
  },
  {
    name: 'Tetrarch',
    host: 'mtg.tetrarch.co/servatrice',
    port: '443',
    editable: false,
  },
];

export const getHostPort = (host: Host): { host: string, port: string } => {
  const isLocal = window.location.hostname === 'localhost';

  if (!host) {
    return {
      host: '',
      port: ''
    };
  }

  return {
    host: !isLocal ? host.host : host.localHost || host.host,
    port: !isLocal ? host.port : host.localPort || host.port,
  }
};

export enum KnownHost {
  ROOSTER = 'Rooster',
  TETRARCH = 'Tetrarch',
}

export const KnownHosts = {
  [KnownHost.ROOSTER]: { port: 4748, host: 'server.cockatrice.us', },
  [KnownHost.TETRARCH]: { port: 443, host: 'mtg.tetrarch.co/servatrice' },
}
