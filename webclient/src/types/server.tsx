export interface ServerStatus {
  status: StatusEnum;
  description: string;
}

export enum StatusEnum {
  DISCONNECTED,
  CONNECTING,
  CONNECTED,
  LOGGINGIN,
  LOGGEDIN,
  REGISTERING,
  REGISTERED,
  ACTIVATING_ACCOUNT,
  ACCOUNT_ACTIVATED,
  RECOVERING_PASSWORD,
  DISCONNECTING =  99
}

export enum StatusEnumLabel {
  "Disconnected",
  "Connecting" ,
  "Connected" ,
  "Loggingin",
  "Loggedin",
  "Registering",
  "Registered",
  "ActivatingAccount",
  "AccountActivated",
  "RecoveringPassword",
  "Disconnecting" = 99
}

export class Host {
  id?: number;
  name: string;
  host: string;
  port: string;
  localHost?: string;
  localPort?: string;
  editable: boolean;
}

export const DefaultHosts: Host[] = [
  {
    name: 'Rooster',
    host: 'server.cockatrice.us/servatrice',
    port: '4748',
    localHost: 'server.cockatrice.us',
    editable: false,
  },
  {
    name: 'Tetrarch',
    host: 'mtg.tetrarch.co/servatrice',
    port: '4748',
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
  [KnownHost.TETRARCH]:  { port: 443, host: 'mtg.tetrarch.co/servatrice'},
}

export const getStatusEnumLabel = (statusEnum: number) => {
  if (StatusEnumLabel[statusEnum] !== undefined) {
    return StatusEnumLabel[statusEnum];
  }

  return "Unknown";
};

export interface Log {
  message: string;
  senderId: string;
  senderIp: string;
  senderName: string;
  targetId: string;
  targetName: string;
  targetType: string;
  time: string;
}

export interface LogGroups {
  room: Log[];
  game: Log[];
  chat: Log[];
}
