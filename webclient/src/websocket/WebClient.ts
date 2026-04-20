import { ping } from './commands/session';
import { CLIENT_OPTIONS } from './config';
import { GameEvents } from './events/game';
import { RoomEvents } from './events/room';
import { SessionEvents } from './events/session';
import type { ConnectTarget } from './types/WebClientConfig';
import type { IWebClientRequest } from './types/WebClientRequest';
import type { IWebClientResponse } from './types/WebClientResponse';
import { StatusEnum } from './types/StatusEnum';
import { ProtobufService } from './services/ProtobufService';
import { WebSocketService } from './services/WebSocketService';

export class WebClient {
  private static _instance: WebClient | null = null;

  static get instance(): WebClient {
    if (!WebClient._instance) {
      throw new Error(
        'WebClient has not been initialized. Instantiate it via `new WebClient()` before accessing `WebClient.instance`.'
      );
    }
    return WebClient._instance;
  }

  protobuf: ProtobufService;
  socket: WebSocketService;
  status: StatusEnum;
  private testSocket: WebSocket | null = null;

  constructor(
    public request: IWebClientRequest,
    public response: IWebClientResponse
  ) {
    if (WebClient._instance) {
      throw new Error('WebClient is a singleton and has already been initialized.');
    }

    this.socket = new WebSocketService({
      keepAliveFn: ping,
      onStatusChange: (status, description) => {
        this.response.session.updateStatus(status, description);
        this.updateStatus(status);
      },
      onConnectionFailed: () => {
        this.response.session.connectionFailed();
      },
      reconnect: {
        maxAttempts: 5,
        baseDelayMs: 1000,
        maxDelayMs: 30000,
      },
    });

    this.protobuf = new ProtobufService(
      {
        send: (data) => this.socket.send(data),
        isOpen: () => this.socket.checkReadyState(WebSocket.OPEN),
      },
      { game: GameEvents, room: RoomEvents, session: SessionEvents },
    );

    this.socket.message$.subscribe((message: MessageEvent) => {
      this.protobuf.handleMessageEvent(message);
    });

    WebClient._instance = this;

    this.response.session.initialized();
  }

  public connect(target: ConnectTarget): void {
    this.response.session.connectionAttempted();
    this.socket.connect(target);
  }

  public testConnect(target: ConnectTarget): void {
    // A prior test connection still in flight when the user re-clicks would
    // otherwise leak the socket until its keepalive timeout. Close eagerly.
    if (this.testSocket) {
      this.testSocket.close();
      this.testSocket = null;
    }

    const protocol = window.location.hostname === 'localhost' ? 'ws' : 'wss';
    const { host, port } = target;
    const socket = new WebSocket(`${protocol}://${host}:${port}`);
    socket.binaryType = 'arraybuffer';
    this.testSocket = socket;

    const timeout = setTimeout(() => socket.close(), CLIENT_OPTIONS.keepalive);

    const clearIfActive = () => {
      if (this.testSocket === socket) {
        this.testSocket = null;
      }
    };

    socket.onopen = () => {
      clearTimeout(timeout);
      this.response.session.testConnectionSuccessful();
      socket.close();
      clearIfActive();
    };

    socket.onerror = () => {
      this.response.session.testConnectionFailed();
      clearIfActive();
    };

    socket.onclose = () => {
      clearIfActive();
    };
  }

  public disconnect(): void {
    this.socket.disconnect();
  }

  public updateStatus(status: StatusEnum): void {
    this.status = status;

    if (status === StatusEnum.DISCONNECTED) {
      this.protobuf.resetCommands();
    }
  }

  public get isReconnecting(): boolean {
    return this.status === StatusEnum.RECONNECTING;
  }
}
