import type { StatusEnum } from './enriched';

export interface ServerStatus {
  status: StatusEnum;
  description: string;
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
