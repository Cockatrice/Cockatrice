import { WebsocketTypes } from '@app/websocket/types';

import { AuthenticationRequestImpl } from './AuthenticationRequestImpl';
import { SessionRequestImpl } from './SessionRequestImpl';
import { RoomsRequestImpl } from './RoomsRequestImpl';
import { GameRequestImpl } from './GameRequestImpl';
import { AdminRequestImpl } from './AdminRequestImpl';
import { ModeratorRequestImpl } from './ModeratorRequestImpl';

export { AuthenticationRequestImpl, SessionRequestImpl, RoomsRequestImpl, GameRequestImpl, AdminRequestImpl, ModeratorRequestImpl };

export function createWebClientRequest(): WebsocketTypes.IWebClientRequest {
  return {
    authentication: new AuthenticationRequestImpl(),
    session: new SessionRequestImpl(),
    rooms: new RoomsRequestImpl(),
    game: new GameRequestImpl(),
    admin: new AdminRequestImpl(),
    moderator: new ModeratorRequestImpl(),
  };
}
