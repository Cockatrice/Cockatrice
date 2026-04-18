import { Subject } from 'rxjs';

import { StatusEnum } from '../interfaces/StatusEnum';
import { KeepAliveService } from './KeepAliveService';
import { CLIENT_OPTIONS } from '../config';
import type { ConnectTarget } from '../interfaces/WebClientConfig';

export interface WebSocketServiceConfig {
  keepAliveFn: (pingReceived: () => void) => void;
  onStatusChange: (status: StatusEnum, description: string) => void;
  onConnectionFailed: () => void;
}

export class WebSocketService {
  private socket: WebSocket;

  private config: WebSocketServiceConfig;
  private keepAliveService: KeepAliveService;
  private errorFired = false;

  public message$: Subject<MessageEvent> = new Subject();

  private keepalive: number;

  constructor(config: WebSocketServiceConfig) {
    this.config = config;

    this.keepAliveService = new KeepAliveService(() => this.checkReadyState(WebSocket.OPEN));
    this.keepAliveService.disconnected$.subscribe(() => {
      this.disconnect();
      this.config.onStatusChange(StatusEnum.DISCONNECTED, 'Connection timeout');
    });
  }

  public connect(target: ConnectTarget, protocol: string = 'wss'): void {
    if (window.location.hostname === 'localhost') {
      protocol = 'ws';
    }

    const { host, port } = target;
    this.keepalive = CLIENT_OPTIONS.keepalive;

    this.socket = this.createWebSocket(`${protocol}://${host}:${port}`);
  }

  public disconnect(): void {
    if (this.socket) {
      this.socket.close();
    }
  }

  public checkReadyState(state: number): boolean {
    return this.socket?.readyState === state;
  }

  public send(message: Uint8Array): void {
    if (!this.socket) {
      return;
    }
    this.socket.send(message as unknown as ArrayBufferView);
  }

  private createWebSocket(url: string): WebSocket {
    const socket = new WebSocket(url);
    socket.binaryType = 'arraybuffer';

    const connectionTimer = setTimeout(() => socket.close(), this.keepalive);

    socket.onopen = () => {
      clearTimeout(connectionTimer);
      this.errorFired = false;
      this.config.onStatusChange(StatusEnum.CONNECTED, 'Connected');

      this.keepAliveService.startPingLoop(this.keepalive, (pingReceived: () => void) => {
        this.config.keepAliveFn(pingReceived);
      });
    };

    socket.onclose = () => {
      // dont overwrite failure messages
      if (!this.errorFired) {
        this.config.onStatusChange(StatusEnum.DISCONNECTED, 'Connection Closed');
      }
      this.errorFired = false;
      this.keepAliveService.endPingLoop();
    };

    socket.onerror = () => {
      this.errorFired = true;
      this.config.onStatusChange(StatusEnum.DISCONNECTED, 'Connection Failed');
      this.config.onConnectionFailed();
    };

    socket.onmessage = (event: MessageEvent) => {
      this.message$.next(event);
    }

    return socket;
  }

}
