import { WebClient } from '@app/websocket';
import type { IWebClientRequest } from '@app/websocket';

export { createWebClientResponse } from './response';
export { createWebClientRequest } from './request';

/**
 * UI-facing request surface. Each property is a lazy getter that resolves
 * `WebClient.instance` at call time, so consumers can import this before the
 * singleton is bootstrapped — it only needs to exist by the first actual call.
 *
 * Prefer this over importing `WebClient` directly: it keeps UI code free of
 * transport-layer names and makes `@app/websocket` an internal detail of the
 * `api` layer.
 */
export const request: IWebClientRequest = {
  get authentication() {
    return WebClient.instance.request.authentication;
  },
  get session() {
    return WebClient.instance.request.session;
  },
  get rooms() {
    return WebClient.instance.request.rooms;
  },
  get admin() {
    return WebClient.instance.request.admin;
  },
  get moderator() {
    return WebClient.instance.request.moderator;
  },
  get game() {
    return WebClient.instance.request.game;
  },
};
