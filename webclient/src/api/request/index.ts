import type { IWebClientRequest } from '@app/websocket';

import { AuthenticationRequestImpl } from './AuthenticationRequestImpl';
import { SessionRequestImpl } from './SessionRequestImpl';
import { RoomsRequestImpl } from './RoomsRequestImpl';
import { AdminRequestImpl } from './AdminRequestImpl';
import { ModeratorRequestImpl } from './ModeratorRequestImpl';

export { AuthenticationRequestImpl } from './AuthenticationRequestImpl';
export { SessionRequestImpl } from './SessionRequestImpl';
export { RoomsRequestImpl } from './RoomsRequestImpl';
export { AdminRequestImpl } from './AdminRequestImpl';
export { ModeratorRequestImpl } from './ModeratorRequestImpl';

export function createWebClientRequest(): IWebClientRequest {
  return {
    authentication: new AuthenticationRequestImpl(),
    session: new SessionRequestImpl(),
    rooms: new RoomsRequestImpl(),
    admin: new AdminRequestImpl(),
    moderator: new ModeratorRequestImpl(),
  };
}
