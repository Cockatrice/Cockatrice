import { Subject } from 'rxjs';

export class KeepAliveService {
  private isOpen: () => boolean;

  private keepalivecb: NodeJS.Timeout;
  private lastPingPending: boolean;

  public disconnected$ = new Subject<void>();

  constructor(isOpen: () => boolean) {
    this.isOpen = isOpen;
  }

  public startPingLoop(interval: number, ping: (onPong: () => void) => void): void {
    this.endPingLoop();
    this.keepalivecb = setInterval(() => {
      // check if the previous ping got no reply
      if (this.lastPingPending) {
        this.disconnected$.next();
      }

      // stop the ping loop if we"re disconnected
      if (!this.isOpen()) {
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
