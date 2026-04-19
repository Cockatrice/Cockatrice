vi.mock('@app/store', () => ({
  GameDispatch: { clearStore: vi.fn(), gameJoined: vi.fn(), playerPropertiesChanged: vi.fn() },
  RoomsDispatch: { clearStore: vi.fn() },
  ServerDispatch: {
    initialized: vi.fn(),
    clearStore: vi.fn(),
    updateStatus: vi.fn(),
    updateUser: vi.fn(),
  },
}));

import { GameDispatch, RoomsDispatch, ServerDispatch } from '@app/store';
import { WebsocketTypes } from '@app/websocket/types';
import { SessionResponseImpl } from './SessionResponseImpl';

describe('SessionResponseImpl.updateStatus', () => {
  let impl: SessionResponseImpl;

  beforeEach(() => {
    vi.clearAllMocks();
    impl = new SessionResponseImpl();
  });

  it('clears game + rooms + server stores when transitioning to DISCONNECTED', () => {
    impl.updateStatus(WebsocketTypes.StatusEnum.DISCONNECTED, 'gone');
    expect(GameDispatch.clearStore).toHaveBeenCalledTimes(1);
    expect(RoomsDispatch.clearStore).toHaveBeenCalledTimes(1);
    expect(ServerDispatch.clearStore).toHaveBeenCalledTimes(1);
    expect(ServerDispatch.updateStatus).toHaveBeenCalledWith(
      WebsocketTypes.StatusEnum.DISCONNECTED,
      'gone',
    );
  });

  it('does NOT clear stores on non-DISCONNECTED transitions', () => {
    impl.updateStatus(WebsocketTypes.StatusEnum.CONNECTED, 'connected');
    expect(GameDispatch.clearStore).not.toHaveBeenCalled();
    expect(RoomsDispatch.clearStore).not.toHaveBeenCalled();
    expect(ServerDispatch.clearStore).not.toHaveBeenCalled();
    expect(ServerDispatch.updateStatus).toHaveBeenCalledWith(
      WebsocketTypes.StatusEnum.CONNECTED,
      'connected',
    );
  });

  it('does NOT clear stores on LOGGED_IN transition', () => {
    impl.updateStatus(WebsocketTypes.StatusEnum.LOGGED_IN, 'in');
    expect(GameDispatch.clearStore).not.toHaveBeenCalled();
    expect(RoomsDispatch.clearStore).not.toHaveBeenCalled();
    expect(ServerDispatch.clearStore).not.toHaveBeenCalled();
  });
});
