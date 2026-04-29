import { fromBinary, getExtension, hasExtension } from '@bufbuild/protobuf';

import {
  Event_ServerIdentification_ext,
  ServerMessageSchema,
  ServerMessage_MessageType,
} from '@app/generated';

import { ping } from './commands/session';
import { CLIENT_OPTIONS, PROTOCOL_VERSION } from './config';
import { GameEvents } from './events/game';
import { RoomEvents } from './events/room';
import { SessionEvents } from './events/session';
import type { ConnectTarget } from './types/WebClientConfig';
import type { IWebClientRequest } from './types/WebClientRequest';
import type { IWebClientResponse } from './types/WebClientResponse';
import { StatusEnum } from './types/StatusEnum';
import { ProtobufService } from './services/ProtobufService';
import { WebSocketService } from './services/WebSocketService';
import { buildWebSocketUrl } from './utils/buildWebSocketUrl';
import { passwordSaltSupported } from './utils/passwordHasher';

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
    const socket = new WebSocket(buildWebSocketUrl(protocol, target.host, target.port));
    socket.binaryType = 'arraybuffer';
    this.testSocket = socket;

    // "Green" means reachable AND speaking a compatible Cockatrice protocol.
    // Waiting for Event_ServerIdentification lets us read the hashed-password
    // capability before the user ever logs in. The bitmask is resolved here
    // (the websocket layer owns protocol details) so downstream consumers
    // receive a domain-level boolean instead of a raw integer.
    let resolved = false;
    const resolve = (ok: boolean, supportsHashedPassword = false): void => {
      if (resolved) {
        return;
      }
      resolved = true;
      clearTimeout(timeout);
      // Suppress dispatches from a superseded socket — a newer test has
      // already taken over and we'd race a stale result into its pending-ref.
      if (this.testSocket === socket) {
        if (ok) {
          this.response.session.testConnectionSuccessful(supportsHashedPassword);
        } else {
          this.response.session.testConnectionFailed();
        }
        this.testSocket = null;
      }
      socket.close();
    };

    const timeout = setTimeout(() => resolve(false), CLIENT_OPTIONS.keepalive);

    socket.onmessage = (event: MessageEvent) => {
      try {
        const msg = fromBinary(ServerMessageSchema, new Uint8Array(event.data));
        if (msg.messageType !== ServerMessage_MessageType.SESSION_EVENT) {
          return;
        }
        const sessionEvent = msg.sessionEvent;
        if (!sessionEvent || !hasExtension(sessionEvent, Event_ServerIdentification_ext)) {
          return;
        }
        const ident = getExtension(sessionEvent, Event_ServerIdentification_ext);
        if (ident.protocolVersion !== PROTOCOL_VERSION) {
          resolve(false);
          return;
        }
        resolve(true, passwordSaltSupported(ident.serverOptions));
      } catch {
        resolve(false);
      }
    };

    socket.onerror = () => resolve(false);
    socket.onclose = () => resolve(false);
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
