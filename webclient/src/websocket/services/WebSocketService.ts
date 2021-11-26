import { Subject } from 'rxjs';

import { ServerStatus, StatusEnum, WebSocketConnectOptions } from 'types';

import { KeepAliveService } from './KeepAliveService';
import { WebClient } from '../WebClient';

export class WebSocketService {
  private socket: WebSocket;
  private webClient: WebClient;
  private keepAliveService: KeepAliveService;

  public message$: Subject<MessageEvent> = new Subject();
  public statusChange$: Subject<ServerStatus> = new Subject();

  private status: StatusEnum = StatusEnum.DISCONNECTED;
  private keepalive: number;

  constructor(webClient: WebClient) {
    this.webClient = webClient;

    this.keepAliveService = new KeepAliveService(this);
    this.keepAliveService.disconnected$.subscribe(() => {
      this.disconnect();
      this.updateStatus(StatusEnum.DISCONNECTED, 'Connection timeout');
    });
  }

  public connect(options: WebSocketConnectOptions, protocol: string = 'wss'): void {
    if (window.location.hostname === 'localhost') {
      protocol = 'ws';
    }

    const { host, port, keepalive } = options;
    this.keepalive = keepalive;

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

  public send(message): void {
    this.socket.send(message);
  }

  public updateStatus(status: StatusEnum, description: string): void {
    this.status = status;
    this.statusChange$.next({ status, description });
  }

  private createWebSocket(url: string): WebSocket {
    const socket = new WebSocket(url);
    socket.binaryType = 'arraybuffer';

    socket.onopen = () => {
      this.updateStatus(StatusEnum.CONNECTED, 'Connected');

      this.keepAliveService.startPingLoop(this.keepalive, (pingReceived: Function) => {
        this.webClient.keepAlive(pingReceived);
      });
    };

    socket.onclose = () => {
      // dont overwrite failure messages
      if (this.status !== StatusEnum.DISCONNECTED) {
        this.updateStatus(StatusEnum.DISCONNECTED, 'Connection Closed');
      }

      this.keepAliveService.endPingLoop();
    };

    socket.onerror = () => {
      this.updateStatus(StatusEnum.DISCONNECTED, 'Connection Failed');
    };

    socket.onmessage = (event: MessageEvent) => {
      this.message$.next(event);
    }

    return socket;
  }
}
