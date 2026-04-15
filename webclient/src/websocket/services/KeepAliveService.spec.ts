import { KeepAliveService } from './KeepAliveService';
import { WebSocketService } from './WebSocketService';

type KeepAliveInternal = KeepAliveService & {
  keepalivecb: NodeJS.Timeout;
  lastPingPending: boolean;
};

vi.mock('./WebSocketService');

describe('KeepAliveService', () => {
  let service: KeepAliveService;
  let mockSocket: { checkReadyState: ReturnType<typeof vi.fn> };

  beforeEach(() => {
    vi.useFakeTimers();

    mockSocket = { checkReadyState: vi.fn().mockReturnValue(true) };
    service = new KeepAliveService(mockSocket as unknown as WebSocketService);
  });

  it('should create', () => {
    expect(service).toBeDefined();
  });

  describe('startPingLoop', () => {
    let resolvePing;
    let interval;
    let promise;
    let ping;
    let checkReadyStateSpy;

    beforeEach(() => {
      interval = 100;
      promise = new Promise(resolve => resolvePing = resolve);
      ping = (done) => promise.then(done);

      checkReadyStateSpy = vi.spyOn(mockSocket, 'checkReadyState');
      checkReadyStateSpy.mockImplementation(() => true);

      service.startPingLoop(interval, ping);
      vi.advanceTimersByTime(interval);
    });

    it('should start ping loop', () => {
      expect((service as KeepAliveInternal).keepalivecb).toBeDefined();
      expect((service as KeepAliveInternal).lastPingPending).toBeTruthy();
    });

    it('should call ping callback when done', () => {
      resolvePing();

      return promise.then(() => {
        expect((service as KeepAliveInternal).lastPingPending).toBeFalsy();
      });
    });

    it('should fire disconnected$ if lastPingPending is still true', () => {
      vi.spyOn(service.disconnected$, 'next').mockImplementation(() => {});
      vi.advanceTimersByTime(interval);

      expect(service.disconnected$.next).toHaveBeenCalled();
    });

    it('should endPingLoop if socket is not open', () => {
      vi.spyOn(service, 'endPingLoop').mockImplementation(() => {});
      checkReadyStateSpy.mockImplementation(() => false);

      resolvePing();
      vi.advanceTimersByTime(interval);

      expect(service.endPingLoop).toHaveBeenCalled();
    });
  });
});
