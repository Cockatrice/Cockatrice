import { KeepAliveService } from './KeepAliveService';

type KeepAliveInternal = KeepAliveService & {
  keepalivecb: NodeJS.Timeout;
  lastPingPending: boolean;
};

describe('KeepAliveService', () => {
  let service: KeepAliveService;
  let mockIsOpen: ReturnType<typeof vi.fn>;

  beforeEach(() => {
    vi.useFakeTimers();

    mockIsOpen = vi.fn().mockReturnValue(true);
    service = new KeepAliveService(mockIsOpen);
  });

  it('should create', () => {
    expect(service).toBeDefined();
  });

  describe('startPingLoop', () => {
    let resolvePing;
    let interval;
    let promise;
    let ping;

    beforeEach(() => {
      interval = 100;
      promise = new Promise(resolve => resolvePing = resolve);
      ping = (done) => promise.then(done);

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
      mockIsOpen.mockReturnValue(false);

      resolvePing();
      vi.advanceTimersByTime(interval);

      expect(service.endPingLoop).toHaveBeenCalled();
    });

    it('should clear previous interval when startPingLoop is called again', () => {
      const clearSpy = vi.spyOn(globalThis, 'clearInterval');
      const previousCb = (service as KeepAliveInternal).keepalivecb;

      service.startPingLoop(interval, ping);

      expect(clearSpy).toHaveBeenCalledWith(previousCb);
    });
  });
});
