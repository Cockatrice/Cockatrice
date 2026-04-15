import { App, Enriched } from '@app/types';

import { ProtobufService } from './services/ProtobufService';
import { WebSocketService } from './services/WebSocketService';
import { ping } from './commands/session';

import { GameDispatch } from '@app/store';
import { RoomPersistence, SessionPersistence } from './persistence';

export class WebClient {
  public socket: WebSocketService;
  public protobuf: ProtobufService;

  public options: Enriched.WebSocketConnectOptions | null = null;
  public status: App.StatusEnum;

  constructor() {
    this.socket = new WebSocketService({
      keepAliveFn: (cb) => ping(cb),
    });

    this.protobuf = new ProtobufService({
      send: (data) => this.socket.send(data),
      isOpen: () => this.socket.checkReadyState(WebSocket.OPEN),
    });

    this.socket.message$.subscribe((message: MessageEvent) => {
      this.protobuf.handleMessageEvent(message);
    });

    SessionPersistence.initialized();

    if (import.meta.env.MODE !== 'test') {
      console.log(this);
    }
  }

  public connect(options: Enriched.WebSocketConnectOptions) {
    SessionPersistence.connectionAttempted();
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
      this.clearStores();
    }
  }

  private clearStores() {
    GameDispatch.clearStore();
    RoomPersistence.clearStore();
    SessionPersistence.clearStore();
  }
}

const webClient = new WebClient();

export default webClient;
