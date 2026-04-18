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
