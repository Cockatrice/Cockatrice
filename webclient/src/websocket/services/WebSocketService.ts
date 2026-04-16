import { Subject } from 'rxjs';

import { App, Enriched } from '@app/types';

import { KeepAliveService } from './KeepAliveService';
import { CLIENT_OPTIONS } from '../config';
import { IWebClientResponse } from '../interfaces';

export interface WebSocketServiceConfig {
  keepAliveFn: (pingReceived: () => void) => void;
  response: IWebClientResponse;
  onStatusChange: (status: App.StatusEnum, description: string) => void;
}

export class WebSocketService {
  private socket: WebSocket;

  private config: WebSocketServiceConfig;
  private response: IWebClientResponse;
  private keepAliveService: KeepAliveService;
  private errorFired = false;

  public message$: Subject<MessageEvent> = new Subject();

  private keepalive: number;

  constructor(config: WebSocketServiceConfig) {
    this.config = config;
    this.response = config.response;

    this.keepAliveService = new KeepAliveService(() => this.checkReadyState(WebSocket.OPEN));
    this.keepAliveService.disconnected$.subscribe(() => {
      this.disconnect();
      this.config.onStatusChange(App.StatusEnum.DISCONNECTED, 'Connection timeout');
    });
  }

  public connect(options: Enriched.WebSocketConnectOptions, protocol: string = 'wss'): void {
    if (window.location.hostname === 'localhost') {
      protocol = 'ws';
    }

    const { host, port } = options;
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
    this.socket.send(message as unknown as ArrayBufferView);
  }

  private createWebSocket(url: string): WebSocket {
    const socket = new WebSocket(url);
    socket.binaryType = 'arraybuffer';

    const connectionTimer = setTimeout(() => socket.close(), this.keepalive);

    socket.onopen = () => {
      clearTimeout(connectionTimer);
      this.errorFired = false;
      this.config.onStatusChange(App.StatusEnum.CONNECTED, 'Connected');

      this.keepAliveService.startPingLoop(this.keepalive, (pingReceived: () => void) => {
        this.config.keepAliveFn(pingReceived);
      });
    };

    socket.onclose = () => {
      // dont overwrite failure messages
      if (!this.errorFired) {
        this.config.onStatusChange(App.StatusEnum.DISCONNECTED, 'Connection Closed');
      }
      this.errorFired = false;
      this.keepAliveService.endPingLoop();
    };

    socket.onerror = () => {
      this.errorFired = true;
      this.config.onStatusChange(App.StatusEnum.DISCONNECTED, 'Connection Failed');
      this.response.session.connectionFailed();
    };

    socket.onmessage = (event: MessageEvent) => {
      this.message$.next(event);
    }

    return socket;
  }

}
