import { App, Enriched } from '@app/types';

import { ProtobufService } from './services/ProtobufService';
import { WebSocketService } from './services/WebSocketService';
import { ping } from './commands/session';
import { IWebClientResponse, IWebClientRequest } from './interfaces';

export class WebClient {
  private static _instance: WebClient | null = null;

  public static get instance(): WebClient {
    if (!WebClient._instance) {
      throw new Error(
        'WebClient has not been initialized. Instantiate it via `new WebClient(response, request)` before accessing `WebClient.instance`.'
      );
    }
    return WebClient._instance;
  }

  public socket: WebSocketService;
  public protobuf: ProtobufService;
  public response: IWebClientResponse;
  public request: IWebClientRequest;

  public options: Enriched.WebSocketConnectOptions | null = null;
  public status: App.StatusEnum;

  constructor(response: IWebClientResponse, request: IWebClientRequest) {
    if (WebClient._instance) {
      throw new Error('WebClient is a singleton and has already been initialized.');
    }

    this.response = response;
    this.request = request;

    this.socket = new WebSocketService({
      keepAliveFn: (cb) => ping(cb),
      response,
      onStatusChange: (status, description) => {
        this.response.session.updateStatus(status, description);
        this.updateStatus(status);
      },
    });

    this.protobuf = new ProtobufService({
      send: (data) => this.socket.send(data),
      isOpen: () => this.socket.checkReadyState(WebSocket.OPEN),
    });

    this.socket.message$.subscribe((message: MessageEvent) => {
      this.protobuf.handleMessageEvent(message);
    });

    WebClient._instance = this;

    this.response.session.initialized();

    if (import.meta.env.MODE !== 'test') {
      console.log(this);
    }
  }

  public connect(options: Enriched.WebSocketConnectOptions) {
    this.response.session.connectionAttempted();
    this.options = options;
    this.socket.connect(options);
  }

  public testConnect(options: Enriched.WebSocketConnectOptions) {
    this.socket.testConnect(options);
  }

  public disconnect() {
    this.socket.disconnect();
  }

  public updateStatus(status: App.StatusEnum) {
    this.status = status;

    if (status === App.StatusEnum.DISCONNECTED) {
      this.protobuf.resetCommands();
    }
  }
}
