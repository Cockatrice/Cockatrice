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
  // Captured from Event_ServerIdentification.serverOptions during test connection.
  // `undefined` = never tested; `true`/`false` = confirmed. UI gates the
  // Remember Password and Auto Connect checkboxes on this.
  supportsHashedPassword?: boolean;
}
