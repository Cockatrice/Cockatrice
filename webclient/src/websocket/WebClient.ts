import { StatusEnum, WebSocketConnectOptions } from 'types';

import { ProtobufService } from './services/ProtobufService';
import { WebSocketService } from './services/WebSocketService';

import { RoomPersistence, SessionPersistence } from './persistence';

export class WebClient {
  public socket = new WebSocketService(this);
  public protobuf = new ProtobufService(this);

  public protocolVersion = 14;
  public clientConfig = {
    clientid: 'webatrice',
    clientver: 'webclient-1.0 (2019-10-31)',
    clientfeatures: [
      'client_id',
      'client_ver',
      'feature_set',
      'room_chat_history',
      'client_warnings',
      /* unimplemented features */
      'forgot_password',
      'idle_client',
      'mod_log_lookup',
      'user_ban_history',
      // satisfy server reqs for POC
      'websocket',
      '2.7.0_min_version',
      '2.8.0_min_version'
    ]
  };

  public clientOptions = {
    autojoinrooms: true,
    keepalive: 5000
  };

  public options: WebSocketConnectOptions;
  public status: StatusEnum;

  public connectionAttemptMade = false;

  constructor() {
    this.socket.message$.subscribe((message: MessageEvent) => {
      this.protobuf.handleMessageEvent(message);
    });

    if (process.env.NODE_ENV !== 'test') {
      console.log(this);
    }
  }

  public connect(options: WebSocketConnectOptions) {
    this.connectionAttemptMade = true;
    this.options = options;
    this.socket.connect(options);
  }

  public testConnect(options: WebSocketConnectOptions) {
    this.socket.testConnect(options);
  }

  public disconnect() {
    this.socket.disconnect();
  }

  public updateStatus(status: StatusEnum) {
    this.status = status;

    if (status === StatusEnum.DISCONNECTED) {
      this.protobuf.resetCommands();
      this.clearStores();
    }
  }

  public keepAlive(pingReceived: Function) {
    this.protobuf.sendKeepAliveCommand(pingReceived);
  }

  private clearStores() {
    RoomPersistence.clearStore();
    SessionPersistence.clearStore();
  }
}

const webClient = new WebClient();

export default webClient;
