import { ping } from './commands/session';
import { CLIENT_OPTIONS } from './config';
import type {
  ConnectTarget,
  IWebClientRequest,
  IWebClientResponse,
} from './interfaces';
import { StatusEnum } from './interfaces';
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
    });

    this.protobuf = new ProtobufService(
      {
        send: (data) => this.socket.send(data),
        isOpen: () => this.socket.checkReadyState(WebSocket.OPEN),
      }
    );

    this.socket.message$.subscribe((message: MessageEvent) => {
      this.protobuf.handleMessageEvent(message);
    });

    WebClient._instance = this;

    this.response.session.initialized();
  }

  public connect(target: ConnectTarget) {
    this.response.session.connectionAttempted();
    this.socket.connect(target);
  }

  public testConnect(target: ConnectTarget) {
    const protocol = window.location.hostname === 'localhost' ? 'ws' : 'wss';
    const { host, port } = target;
    const socket = new WebSocket(`${protocol}://${host}:${port}`);
    socket.binaryType = 'arraybuffer';

    const timeout = setTimeout(() => socket.close(), CLIENT_OPTIONS.keepalive);

    socket.onopen = () => {
      clearTimeout(timeout);
      this.response.session.testConnectionSuccessful();
      socket.close();
    };

    socket.onerror = () => {
      this.response.session.testConnectionFailed();
    };

    socket.onclose = () => {};
  }

  public disconnect() {
    this.socket.disconnect();
  }

  public updateStatus(status: StatusEnum) {
    this.status = status;

    if (status === StatusEnum.DISCONNECTED) {
      this.protobuf.resetCommands();
    }
  }
}
