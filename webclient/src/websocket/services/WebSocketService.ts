import { Observable, Subject } from 'rxjs';

import { StatusEnum } from '../types/StatusEnum';
import { KeepAliveService } from './KeepAliveService';
import { CLIENT_OPTIONS } from '../config';
import type { ConnectTarget } from '../types/WebClientConfig';
import { buildWebSocketUrl } from '../utils/buildWebSocketUrl';

export interface ReconnectConfig {
  maxAttempts: number;
  baseDelayMs: number;
  maxDelayMs: number;
}

export interface WebSocketServiceConfig {
  keepAliveFn: (pingReceived: () => void) => void;
  onStatusChange: (status: StatusEnum, description: string) => void;
  onConnectionFailed: () => void;
  /** Opt-in automatic reconnect on unexpected socket close. */
  reconnect?: ReconnectConfig;
}

export class WebSocketService {
  private socket: WebSocket | null = null;

  private config: WebSocketServiceConfig;
  private keepAliveService: KeepAliveService;
  private hasReportedError = false;

  private readonly messageSubject = new Subject<MessageEvent>();
  public readonly message$: Observable<MessageEvent> = this.messageSubject.asObservable();

  private keepalive: number = CLIENT_OPTIONS.keepalive;

  private lastTarget: ConnectTarget | null = null;
  private lastProtocol: string | null = null;

  private intentionalDisconnect = false;
  /**
   * True while `connect()` is cycling a prior socket out to bring a fresh one up.
   * Suppresses the `DISCONNECTED` status emission the orphan socket's onclose
   * would otherwise fire — that would clobber the `connectionAttempted()` we
   * just dispatched at the WebClient layer.
   */
  private retiringForReconnect = false;
  private reconnectAttempts = 0;
  private reconnectTimer: ReturnType<typeof setTimeout> | null = null;
  /** Flips true on the first successful `onopen`. Gates reconnect — we never retry a connection that never established. */
  private hasEverOpened = false;

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

    // Retire any prior socket cleanly. The `retiringForReconnect` flag both
    // suppresses reconnect scheduling for the orphan socket AND suppresses the
    // DISCONNECTED status emission that would otherwise reset
    // `connectionAttemptMade` set by the caller a moment ago.
    this.retiringForReconnect = true;
    this.clearReconnectTimer();
    this.closeActiveSocket();
    this.retiringForReconnect = false;

    this.lastTarget = target;
    this.lastProtocol = protocol;
    this.intentionalDisconnect = false;
    this.reconnectAttempts = 0;
    this.hasEverOpened = false;
    this.keepalive = CLIENT_OPTIONS.keepalive;

    const { host, port } = target;
    this.socket = this.createWebSocket(buildWebSocketUrl(protocol as 'ws' | 'wss', host, port));
  }

  public disconnect(): void {
    this.intentionalDisconnect = true;
    this.clearReconnectTimer();
    this.closeActiveSocket();
  }

  public checkReadyState(state: number): boolean {
    return this.socket?.readyState === state;
  }

  public send(message: Uint8Array): void {
    if (!this.socket) {
      return;
    }
    if (this.socket.readyState !== WebSocket.OPEN) {
      // Match desktop's TCP-queued semantics conservatively: drop with a warn rather
      // than throw. Upstream code treats send as fire-and-forget under these states.
      console.warn('[WebSocketService] send() skipped: socket not OPEN', this.socket.readyState);
      return;
    }
    this.socket.send(message as BufferSource);
  }

  private createWebSocket(url: string): WebSocket {
    const socket = new WebSocket(url);
    socket.binaryType = 'arraybuffer';

    const connectionTimer = setTimeout(() => socket.close(), this.keepalive);
    const clearConnectionTimer = (): void => clearTimeout(connectionTimer);

    socket.onopen = () => {
      this.hasEverOpened = true;
      clearConnectionTimer();
      this.hasReportedError = false;
      this.reconnectAttempts = 0;
      this.config.onStatusChange(StatusEnum.CONNECTED, 'Connected');

      this.keepAliveService.startPingLoop(this.keepalive, (pingReceived: () => void) => {
        this.config.keepAliveFn(pingReceived);
      });
    };

    socket.onclose = () => {
      clearConnectionTimer();
      this.keepAliveService.endPingLoop();

      if (this.shouldAttemptReconnect()) {
        this.scheduleReconnect();
        return;
      }

      // Orphan socket retired by a fresh connect() call — the new socket is
      // already being wired up; don't fire a DISCONNECTED status that would
      // race the just-dispatched connectionAttempted.
      if (this.retiringForReconnect) {
        this.hasReportedError = false;
        return;
      }

      // @critical onerror + onclose both fire on failed connects; don't overwrite the richer error status.
      // See .github/instructions/webclient.instructions.md#websocket-lifecycle.
      if (!this.hasReportedError) {
        this.config.onStatusChange(StatusEnum.DISCONNECTED, 'Connection Closed');
      }
      this.hasReportedError = false;
    };

    socket.onerror = () => {
      clearConnectionTimer();
      this.hasReportedError = true;
      this.config.onStatusChange(StatusEnum.DISCONNECTED, 'Connection Failed');
      this.config.onConnectionFailed();
    };

    socket.onmessage = (event: MessageEvent) => {
      this.messageSubject.next(event);
    };

    return socket;
  }

  private shouldAttemptReconnect(): boolean {
    if (this.intentionalDisconnect) {
      return false;
    }
    if (this.retiringForReconnect) {
      return false;
    }
    // Suppress reconnect when the connection never established — onerror already
    // reported DISCONNECTED and we don't want to thrash against a dead endpoint.
    if (this.hasReportedError) {
      return false;
    }
    // Only retry once we have proof the endpoint was reachable at least once.
    // The initial connect path falls through to DISCONNECTED on failure.
    if (!this.hasEverOpened) {
      return false;
    }
    const cfg = this.config.reconnect;
    if (!cfg || cfg.maxAttempts <= 0) {
      return false;
    }
    return this.reconnectAttempts < cfg.maxAttempts;
  }

  private scheduleReconnect(): void {
    const cfg = this.config.reconnect!;
    const attempt = this.reconnectAttempts;
    const delay = Math.min(cfg.baseDelayMs * Math.pow(2, attempt), cfg.maxDelayMs);
    this.reconnectAttempts += 1;

    this.config.onStatusChange(
      StatusEnum.RECONNECTING,
      `Reconnecting (attempt ${this.reconnectAttempts}/${cfg.maxAttempts})`,
    );

    this.reconnectTimer = setTimeout(() => {
      this.reconnectTimer = null;
      if (this.intentionalDisconnect || !this.lastTarget || !this.lastProtocol) {
        return;
      }
      const { host, port } = this.lastTarget;
      this.socket = this.createWebSocket(
        buildWebSocketUrl(this.lastProtocol as 'ws' | 'wss', host, port),
      );
    }, delay);
  }

  private clearReconnectTimer(): void {
    if (this.reconnectTimer !== null) {
      clearTimeout(this.reconnectTimer);
      this.reconnectTimer = null;
    }
  }

  private closeActiveSocket(): void {
    if (this.socket) {
      this.socket.close();
      this.socket = null;
    }
  }

}
