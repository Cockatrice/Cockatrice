import { Subject } from 'rxjs';

import { WebSocketService } from './WebSocketService';

export class KeepAliveService {
  private socket: WebSocketService;

  private keepalivecb: NodeJS.Timeout;
  private lastPingPending: boolean;

  public disconnected$ = new Subject<void>();

  constructor(socket: WebSocketService) {
    this.socket = socket;
  }

  public startPingLoop(interval: number, ping: Function): void {
    this.keepalivecb = setInterval(() => {
      // check if the previous ping got no reply
      if (this.lastPingPending) {
        this.disconnected$.next();
      }

      // stop the ping loop if we"re disconnected
      if (!this.socket.checkReadyState(WebSocket.OPEN)) {
        this.endPingLoop();
        return;
      }

      this.lastPingPending = true;
      ping(() => this.lastPingPending = false);
    }, interval);
  }

  public endPingLoop() {
    clearInterval(this.keepalivecb);
    this.keepalivecb = null;
    this.lastPingPending = false;
  }
}
