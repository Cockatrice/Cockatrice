import { Subject } from 'rxjs';

import { StatusEnum, WebSocketConnectOptions } from 'types';

import { KeepAliveService } from './KeepAliveService';
import { WebClient } from '../WebClient';
import { SessionPersistence } from '../persistence';
import { updateStatus } from '../commands/session';

export class WebSocketService {
  private socket: WebSocket;
  private testSocket: WebSocket;

  private webClient: WebClient;
  private keepAliveService: KeepAliveService;

  public message$: Subject<MessageEvent> = new Subject();

  private keepalive: number;

  constructor(webClient: WebClient) {
    this.webClient = webClient;

    this.keepAliveService = new KeepAliveService(this);
    this.keepAliveService.disconnected$.subscribe(() => {
      this.disconnect();
      updateStatus(StatusEnum.DISCONNECTED, 'Connection timeout');
    });
  }

  public connect(options: WebSocketConnectOptions, protocol: string = 'wss'): void {
    if (window.location.hostname === 'localhost') {
      protocol = 'ws';
    }

    const { host, port } = options;
    this.keepalive = this.webClient.clientOptions.keepalive;

    this.socket = this.createWebSocket(`${protocol}://${host}:${port}`);
  }

  public testConnect(options: WebSocketConnectOptions, protocol: string = 'wss'): void {
    if (window.location.hostname === 'localhost') {
      protocol = 'ws';
    }

    const { host, port } = options;

    this.testWebSocket(`${protocol}://${host}:${port}`);
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

  private createWebSocket(url: string): WebSocket {
    const socket = new WebSocket(url);
    socket.binaryType = 'arraybuffer';

    const connectionTimer = setTimeout(() => socket.close(), this.keepalive);

    socket.onopen = () => {
      clearTimeout(connectionTimer);
      updateStatus(StatusEnum.CONNECTED, 'Connected');

      this.keepAliveService.startPingLoop(this.keepalive, (pingReceived: Function) => {
        this.webClient.keepAlive(pingReceived);
      });
    };

    socket.onclose = () => {
      // dont overwrite failure messages
      if (this.webClient.status !== StatusEnum.DISCONNECTED) {
        updateStatus(StatusEnum.DISCONNECTED, 'Connection Closed');
      }

      this.keepAliveService.endPingLoop();
    };

    socket.onerror = () => {
      updateStatus(StatusEnum.DISCONNECTED, 'Connection Failed');
      SessionPersistence.connectionFailed();
    };

    socket.onmessage = (event: MessageEvent) => {
      this.message$.next(event);
    }

    return socket;
  }

  private testWebSocket(url: string): void {
    if (this.testSocket) {
      this.testSocket.onerror = null;
      this.testSocket.close();
    }

    const socket = new WebSocket(url);
    socket.binaryType = 'arraybuffer';

    const connectionTimer = setTimeout(() => socket.close(), this.webClient.clientOptions.keepalive);

    socket.onopen = () => {
      clearTimeout(connectionTimer);
      SessionPersistence.testConnectionSuccessful();
      socket.close();
    };

    socket.onerror = () => {
      SessionPersistence.testConnectionFailed();
    };

    socket.onclose = () => {
      this.testSocket = null;
    }

    this.testSocket = socket;
  }
}
